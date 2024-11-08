#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

/**
 * 使用 mmap 的匿名共享内存，来实现父子进程之间的通信。
 */
int main() {
  pid_t pid;
  char *shm =
      mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

  if (!((pid = fork()))) { // returning 0 means it is the child process.
    sleep(1);
    printf("Child got a message: %s\n", shm);
    sprintf(shm, "%s", "hello, father.");
    exit(0);
  }

  sprintf(shm, "%s", "hello, my child.");
  sleep(2);
  printf("parent got a message: %s\n", shm);
  return 0;
}