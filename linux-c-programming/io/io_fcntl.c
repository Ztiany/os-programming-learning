/*
 * =======================================
 *  演示 Linux 文件操作。
 * =======================================
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define MSG_TRY "try again\n"

/*
 * 可以用 fcntl 函数改变一个已打开的文件的属性，可以重新设置读、写、追加、非阻塞等标志（这些标志称
 * 为 File Status Flag），而不必重新 open 文件。
 */
void set_file_status() {
  char buf[10];
  int n;
  int flags;
  flags = fcntl(STDIN_FILENO, F_GETFL);
  flags |= O_NONBLOCK;
  if (fcntl(STDIN_FILENO, F_SETFL, flags) == -1) {
    perror("fcntl");
    exit(1);
  }

tryagain:
  n = read(STDIN_FILENO, buf, 10);
  if (n < 0) {
    if (errno == EAGAIN) {
      sleep(1);
      write(STDOUT_FILENO, MSG_TRY, strlen(MSG_TRY));
      printf("polling...\n");
      goto tryagain;
    }
    perror("read stdin");
    exit(1);
  }
  write(STDOUT_FILENO, buf, n);
}

/*
 以下程序通过命令行的第一个参数指定一个文件描述符，同时利用 Shell 的重定向功能在该描述符上打开文件，
 然后用 fcntl 的 F_GETFL 命令取出 File Status Flag 并打印。

    示例 1 参数：./a.out 0 < /dev/tty

        控制终端（/dev/tty，如果当前进程有控制终端（Controlling Terminal） 的话，那么
        /dev/tty 就是当前进程的控制终端的设备特殊文件。）

        Shell 在执行 a.out 时将它的标准输入重定向到 /dev/tty，并且是只读的。argv[1] 是 0，
        因此取出文件描述符 0（也就是标准输入）的 File Status Flag， 用掩码 O_ACCMODE 取出它
        的读写位，结果是 O_RDONLY。

        注意，Shell 的重定向语法不属于程序的命令行参数，这个命行只有两个参数：

            argv[0] 是"./a.out"
            argv[1] 是"0"

        重定向由 Shell 解释，在启动程序时已经生效，程序在运行时并不知道标准输入被重定向了。

    示例 2 参数：./a.out 1 > temp.foo

        Shell 在执行 a.out 时将它的标准输出重定向到文件 temp.foo，并且是只写的。程序取出文件描
        述符 1 的 File Status Flag，发现是只写的，于是打印 write only，但是打印不到屏幕上而
        是打印到 temp.foo 这个文件中了。
*/
void dump_file_status(int argc, char const *argv[]) {

  for (size_t i = 0; i < argc; i++) {
    printf("args index %zd is %s \n", i, argv[i]);
  }

  int val;
  if (argc != 2) {
    fputs("usage: a.out <descriptor#>\n", stderr);
    exit(1);
  }
  if ((val = fcntl(atoi(argv[1]), F_GETFL)) < 0) {
    printf("fcntl error for fd %d\n", atoi(argv[1]));
    exit(1);
  }
  switch (val & O_ACCMODE) {
  case O_RDONLY:
    printf("read only");
    break;
  case O_WRONLY:
    printf("write only");
    break;
  case O_RDWR:
    printf("read write");
    break;
  default:
    fputs("invalid access mode\n", stderr);
        exit(1);
    }
    if (val & O_APPEND)
        printf(", append");
    if (val & O_NONBLOCK)
        printf(", nonblocking");
    putchar('\n');
}

int main(int argc, char const *argv[])
{
    // dump_file_status(argc, argv);
    set_file_status();
    return 0;
}
