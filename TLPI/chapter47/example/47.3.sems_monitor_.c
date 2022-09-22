#include <sys/sem.h>
#include <time.h>
#include <lib/semun.h> /* Definition of semun union */
#include <lib/tlpi_hdr.h>

/*==================================================================================================
 监视信号量的关联数据
 ================================================================================================== */

int main(int argc, char *argv[]) {
    struct semid_ds ds;

    /* Fourth argument for semctl() */
    union semun arg;
    union semun dummy;//有些系统实现总是需要第四个参数，所以传一个 dummy。

    int semid, j;

    if (argc != 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s semid\n", argv[0]);
    semid = getInt(argv[1], 0, "semid");

    //打印信号量的时间信息
    arg.buf = &ds;
    if (semctl(semid, 0, IPC_STAT, arg) == -1)
        errExit("semctl");

    printf("Semaphore changed: %s", ctime(&ds.sem_ctime));
    printf("Last semop(): %s", ctime(&ds.sem_otime));

    /* Display per-semaphore information */
    arg.array = calloc(ds.sem_nsems, sizeof(arg.array[0]));
    if (arg.array == NULL)
        errExit("calloc");
    // 获取所有信号量的下标
    if (semctl(semid, 0, GETALL, arg) == -1)
        errExit("semctl-GETALL");

    //打印所有信号量的信息
    printf("Sem # Value SEMPID SEMNCNT SEMZCNT\n");
    for (j = 0; j < ds.sem_nsems; j++)
        printf("%3d %5d %5d %5d %5d\n", j, arg.array[j],
               semctl(semid, j, GETPID, dummy),
               semctl(semid, j, GETNCNT, dummy),
               semctl(semid, j, GETZCNT, dummy));

    exit(EXIT_SUCCESS);
}
