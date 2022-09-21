#ifndef UNIXP_SYSV_S_SEM_SAMPLE_H
#define UNIXP_SYSV_S_SEM_SAMPLE_H

#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <cerrno>
#include <sys/ipc.h>
#include <sys/sem.h>

/** 一个二元信号量封装【不保证多进程初始化安全】*/
class SimpleTwoSemaphore {

private:

    union semun {  // 用于信号灯操作的共同体。
        int val;
        struct semid_ds *buf;
        unsigned short *array;
    };

    int sem_id;  // 信号灯描述符。

public:
    bool init(key_t key); // 如果信号灯已存在，获取信号灯；如果信号灯不存在，则创建信号灯并初始化。
    bool wait();          // 等待信号灯挂出。
    bool post();          // 挂出信号灯。
    bool destroy();       // 销毁信号灯。
};


bool SimpleTwoSemaphore::init(key_t key) {

    // 获取信号灯。
    if ((sem_id = semget(key, 1, 0640)) == -1) {

        // 如果信号灯不存在，创建它。
        if (errno == 2) {

            if ((sem_id = semget(key, 1, 0640 | IPC_CREAT)) == -1) {
                perror("init 1 semget()");
                return false;
            }

            // 信号灯创建成功后，还需要把它初始化成可用的状态。
            union semun sem_union;
            sem_union.val = 1;
            if (semctl(sem_id, 0, SETVAL, sem_union) < 0) {
                perror("init semctl()");
                return false;
            }

        } else {
            perror("init 2 semget()");
            return false;
        }

    }
    return true;
}

bool SimpleTwoSemaphore::destroy() {
    if (semctl(sem_id, 0, IPC_RMID) == -1) {
        perror("destroy semctl()");
        return false;
    }

    return true;
}

bool SimpleTwoSemaphore::wait() {
    struct sembuf sem_b;
    sem_b.sem_num = 0;
    sem_b.sem_op = -1;
    sem_b.sem_flg = SEM_UNDO;
    if (semop(sem_id, &sem_b, 1) == -1) {
        perror("wait semop()");
        return false;
    }

    return true;
}

bool SimpleTwoSemaphore::post() {

    struct sembuf sem_b;
    sem_b.sem_num = 0;
    sem_b.sem_op = 1;
    sem_b.sem_flg = SEM_UNDO;

    if (semop(sem_id, &sem_b, 1) == -1) {
        perror("post semop()");
        return false;
    }

    return true;
}

#endif //UNIXP_SYSV_S_SEM_SAMPLE_H
