#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/unistd.h>

/*
 * =======================================
 *  演示 System V 共享内存
 * =======================================
 */

struct st_pid {
  int pid;       // 进程编号。
  char name[51]; // 进程名称。
};

int main(int argc, char *argv[]) {
  // 共享内存的标志。
  int shm_id;

  // 获取或者创建共享内存，键值为 0x5005。
  if ((shm_id = shmget(0x5005, sizeof(struct st_pid), 0640 | IPC_CREAT)) ==
      -1) {
    printf("shmget(0x5005) failed\n");
    return EXIT_FAILURE;
  }

  // 用于指向共享内存的结构体变量。
  struct st_pid *st_pid = NULL;

  // 把共享内存连接到当前进程的地址空间。
  if ((st_pid = (struct st_pid *)shmat(shm_id, 0, 0)) == (void *)-1) {
    printf("shmat failed\n");
    return EXIT_FAILURE;
  }
  printf("pid=%d,name=%s\n", st_pid->pid, st_pid->name);

  // 拷贝一个数据到共享内存中
  st_pid->pid = getpid();
  strcpy(st_pid->name, argv[1]);
  printf("pid=%d,name=%s\n", st_pid->pid, st_pid->name);

  sleep(10);

  // 把共享内存从当前进程中分离。
  shmdt(st_pid);

  // 删除共享内存。
  if (shmctl(shm_id, IPC_RMID, 0) == -1) {
        printf("shmctl failed\n");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}