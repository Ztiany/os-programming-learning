/*
 * =======================================
 *  演示 Linux 文件操作。
 * =======================================
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#define MSG_TIMEOUT "timeout\n"
#define MSG_TRY "try again\n"

/*
 下面这个小程序从终端读数据再写回终端。

  读时阻塞：读常规文件是不会阻塞的，不管读多少字节，read 一定会在有限的时间内返回。从终端设备或网络
          读则不一定，如果从终端输入的数据没有换行符，调用 read 读终端设备就会阻塞，如果网络上没
          有接收到数据包，调用 read 从网络读就会阻塞，至于会阻塞多长时间也是不确定的，如果一直没
          有数据到达就一直阻塞在那里。

          同样，写常规文件是不会阻塞的，而向终端设备或网络写则不一定。

  什么是阻塞：当进程调用一个阻塞的系统函数时，该进程被置于睡眠（Sleep）状态，这时内核调度其它进程运
            行，直到该进程等待的事件发生了（比如网络上接收到数据包，或者调用 sleep 指定的睡眠时
            间到了）它才有可能继续运行。与睡眠状态相对的是运行（Running）状态，在 Linux 内核中，
            处于运行状态的进程分为两种情况：

            1. 正在被调度执行。CPU 处于该进程的上下文环境中，程序计数器（eip）里保存着该进程的
            指令地址，通用寄存器里保存着该进程运算过程的中间结果，正在执行该进程的指令，正在读写
            该进程的地址空间。

            2. 就绪状态。该进程不需要等待什么事件发生，随时都可以执行，但 CPU 暂时还在执行另一个
            进程，所以该进程在一个就绪队列中等待被内核调度。系统中可能同时有多个就绪的进程，那么该
            调度谁执行呢？内核的调度算法是基于优先级和时间片的，而且会根据每个进程的运行情况动态调
            整它的优先级和时间片，让每个进程都能比较公平地得到机会执行，同时要兼顾用户体验，不能让
            和用户交互的进程响应太慢。
*/
void blocking_echo() {
  char buffer[1024];
  const int c_read = read(STDIN_FILENO, buffer, 1024);
  if (c_read < 0) {
    perror("read STDIN_FILENO");
    exit(1);
  }
  write(STDOUT_FILENO, buffer, c_read);
}


/*
 非阻塞读：如果在 open 一个设备时指定了 O_NONBLOCK 标志，read/write 就不会阻塞。以 read 为例，
         如果设备暂时没有数据可读就返回 -1，同时置 errno 为 EWOULDBLOCK（或者 EAGAIN，这两个
         宏定义的值相同），表示本来应该阻塞在这里（would block，虚拟语气），事实上并没有阻塞而是
         直接返回错误，调用者应该试着再读一次（again）。这种行为方式称为轮询（Poll），调用者只是
         查询一下，而不是阻塞在这里死等，这样可以同时监视多个设备：

                        while(1) {
                          非阻塞read(设备1);
                          if(设备1有数据到达)
                            处理数据;
                          非阻塞read(设备2);
                          if(设备2有数据到达)
                            处理数据;
                          ...
                        }

 如果 read(设备1) 是阻塞的，那么只要设备 1 没有数据到达就会一直阻塞在设备1的 read 调用上，即使设
 备 2 有数据到达也不能处理，使用非阻塞 I/O 就可以避免设备 2 得不到及时处理。

 非阻塞 I/O 有一个缺点，如果所有设备都一直没有数据到达，调用者需要反复查询做无用功，如果阻塞在那里，
 操作系统可以调度别的进程执行，就不会做无用功了。在使用非阻塞 I/O 时，通常不会在一个 while 循环中
 一直不停地查询（这称为 Tight Loop），而是每延迟等待一会儿来查询一下，以免做太多无用功，在延迟等待
 的时候可以调度其它进程执行。

                          while(1) {
                            非阻塞read(设备1);
                            if(设备1有数据到达)
                              处理数据;
                            非阻塞read(设备2);
                            if(设备2有数据到达)
                              处理数据;
                            ...
                            sleep(n);//睡眠一下
                          }


 这样做的问题是，设备 1 有数据到达时可能不能及时处理，最长需延迟 n 秒才能处理，而且反复查询还是做了
 很多无用功。select 函数可以阻塞地同时监视多个设备，还可以设定阻塞等待的超时时间，从而圆满地解决了
 这个问题。
*/
void nonblocking_echo() {
  char buffer[10];
  int c_read;
  int fd = open("/dev/tty" /* 当前终端 */, O_RDONLY | O_NONBLOCK);
  if (fd < 0)
  {
    perror("open /dev/tty");
    exit(1);
  }

  tryagain:
        c_read = read(fd, buffer, 10);
  if (c_read < 0)
  {
    if (errno == EAGAIN)
    {
      sleep(1);
      write(STDOUT_FILENO, MSG_TRY, strlen(MSG_TRY));
      goto tryagain;
    }
    perror("read /dev/tty");
    exit(1);
  }
  write(STDOUT_FILENO, buffer, c_read);
  close(fd);
}

/*
 * 以下是用非阻塞 I/O 实现等待超时的例子。既保证了超时退出的逻辑又保证了有数据到达时处理延迟较小。
 */
void nonblocking_timeout_echo() {
  char buffer[10];
  int  c_read, i;
  int fd = open("/dev/tty", O_RDONLY | O_NONBLOCK);
  if (fd < 0)
  {
    perror("open /dev/tty");
    exit(1);
  }

  for (i = 0; i < 5; i++)
  {
    c_read = read(fd, buffer, 10);
    if (c_read >= 0)
      break;
    if (errno != EAGAIN)
    {
      perror("read /dev/tty");
      exit(1);
    }
    sleep(1);
    write(STDOUT_FILENO, MSG_TRY, strlen(MSG_TRY));
  }

  if (i == 5)
    write(STDOUT_FILENO, MSG_TIMEOUT, strlen(MSG_TIMEOUT));
  else
    write(STDOUT_FILENO, buffer, c_read);

  close(fd);
}

int main(int argc, const char *argv[]) {
  // blocking_echo();
  //nonblocking_echo();
  nonblocking_timeout_echo();
}
