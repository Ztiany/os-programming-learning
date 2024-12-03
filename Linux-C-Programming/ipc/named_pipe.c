#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/*
 * =======================================
 *  演示使用命名管道进行进程间通信。
 * =======================================
 */

/**
 * 测试管道的文件类型
 */
void dump_pipe_info(const int fd) {
  struct stat st;
  fstat(fd, &st);
  printf("fd is FIFO: %d\n", S_ISFIFO(st.st_mode));
}

int main(int argc, const char *argv[]) {
  int fd[2];
  pid_t pid;
  char line[PIPE_BUF];

  // 创建一个管道，对应两个 fd，一个用于读，一个用于写。
  if (pipe(fd) < 0) {
    printf("pipe error\n");
  }

  dump_pipe_info(fd[0]);

  if ((pid = fork()) < 0) {
    printf("fork error\n");
    exit(EXIT_FAILURE);
  }

  if (pid > 0) { // parent
    close(fd[0]);
    write(fd[1], "hello world\n", 12);
  } else {
    // child
    close(fd[1]);
    const int c_read = read(fd[0], line, PIPE_BUF);
    write(STDOUT_FILENO, line, c_read);
  }

  exit(EXIT_SUCCESS);
}