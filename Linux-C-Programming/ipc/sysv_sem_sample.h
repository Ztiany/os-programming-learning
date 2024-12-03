#ifndef UNIXP_SYSV_SEM_SAMPLE_H
#define UNIXP_SYSV_SEM_SAMPLE_H

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>

/** 一个二元信号量封装【不保证多进程初始化安全】 */
class TwoSemaphore {
private:
  union semun { // 用于信号量操作的共同体。
    int val;
    struct semid_ds *buf;
    unsigned short *array;
  };

  int m_sem_id; // 信号量描述符。

  // 如果把 sem_flg 设置为 SEM_UNDO，操作系统将跟踪进程对信号量的修改情况，
  // 在全部修改过信号量的进程（正常或异常）终止后，操作系统将把信号量恢
  // 复为初始值（就像撤消了全部进程对信号的操作）。
  // 如果信号量用于表示可用资源的数量（不变的），设置为 SEM_UNDO 更合适。
  // 如果信号量用于生产消费者模型，设置为 0 更合适。
  // 注意，网上查到的关于 sem_flg 的用法基本上是错的，一定要自己动手多测试。
  short m_sem_flg;

public:
  TwoSemaphore();

  // 如果信号量已存在，获取信号量；如果信号量不存在，则创建它并初始化为 value。
  bool init(key_t key, unsigned short value = 1, short sem_flg = SEM_UNDO);

  bool P(short sem_op = -1); // 信号量的 P 操作。
  bool V(short sem_op = 1);  // 信号量的 V 操作。

  int value(); // 获取信号量的值，成功返回信号量的值，失败返回-1。

  bool destroy(); // 销毁信号量。

  ~TwoSemaphore();
};

TwoSemaphore::TwoSemaphore() {
  m_sem_id = -1;
  m_sem_flg = SEM_UNDO;
}

// 如果信号量已存在，获取信号量；如果信号量不存在，则创建它并初始化为 value。
bool TwoSemaphore::init(key_t key, unsigned short value, short sem_flg) {
  if (m_sem_id != -1)
    return false;

  m_sem_flg = sem_flg;

  // 信号量的初始化不能直接用
  // semget(key,1,0666|IPC_CREAT)，因为信号量创建后，初始值是 0。

  // 信号量的初始化分三个步骤：
  // 1）获取信号量，如果成功，函数返回。
  // 2）如果失败，则创建信号量。
  // 3) 设置信号量的初始值。

  // 获取信号量。
  if ((m_sem_id = semget(key, 1, 0666)) == -1) {
    // 如果信号量不存在，创建它。
    if (errno == 2) {
      // 用 IPC_EXCL 标志确保只有一个进程创建并初始化信号量，其它进程只能获取。
      if ((m_sem_id = semget(key, 1, 0666 | IPC_CREAT | IPC_EXCL)) == -1) {
        if (errno != EEXIST) {
          perror("init 1 semget()");
          return false;
        }
        if ((m_sem_id = semget(key, 1, 0666)) == -1) {
          perror("init 2 semget()");
          return false;
        }

        return true;
      }

      // 信号量创建成功后，还需要把它初始化成 value。
      union semun sem_union;
      sem_union.val = value; // 设置信号量的初始值。
      if (semctl(m_sem_id, 0, SETVAL, sem_union) < 0) {
        perror("init semctl()");
        return false;
      }
    } else {
      perror("init 3 semget()");
      return false;
    }
  }

  return true;
}

bool TwoSemaphore::P(short sem_op) {
  if (m_sem_id == -1)
    return false;

  struct sembuf sem_b;
  sem_b.sem_num = 0;     // 信号量编号，0 代表第一个信号量。
  sem_b.sem_op = sem_op; // P 操作的 sem_op 必须小于 0。
  sem_b.sem_flg = m_sem_flg;
  if (semop(m_sem_id, &sem_b, 1) == -1) {
    perror("p semop()");
    return false;
  }

  return true;
}

bool TwoSemaphore::V(short sem_op) {
  if (m_sem_id == -1)
    return false;

  struct sembuf sem_b;
  sem_b.sem_num = 0;     // 信号量编号，0 代表第一个信号量。
  sem_b.sem_op = sem_op; // V 操作的 sem_op 必须大于 0。
  sem_b.sem_flg = m_sem_flg;
    if (semop(m_sem_id, &sem_b, 1) == -1) {
        perror("V semop()");
        return false;
    }

    return true;
}

// 获取信号量的值，成功返回信号量的值，失败返回 -1。
int TwoSemaphore::value() {
    return semctl(m_sem_id, 0, GETVAL);
}

bool TwoSemaphore::destroy() {
    if (m_sem_id == -1) return false;

    if (semctl(m_sem_id, 0, IPC_RMID) == -1) {
        perror("destroy semctl()");
        return false;
    }

    return true;
}

TwoSemaphore::~TwoSemaphore() {
}

#endif //UNIXP_SYSV_SEM_SAMPLE_H
