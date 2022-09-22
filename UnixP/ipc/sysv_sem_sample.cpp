#include "sysv_sem_sample.h"

/*==================================================================================================
 演示 System V 信号量【示例 2】
 ================================================================================================== */

/**
 * 演示步骤：
 *
 * 1. 运行该程序，它会创建键值为 5000 的信号灯，并持有锁；
 * 2. 立即再运行一次该程序，它会获取键值为 5000 的信号灯，并等待锁；
 * 3. 当第一次运行的程序 sleep 完 20 秒之后，释放锁，第二个运行的程序将获得锁；
 */
int main(int argc, char *argv[]) {
    TwoSemaphore sem;

    // 初始信号灯。
    if (!sem.init(0x5000)) {
        printf("sem.init failed.\n");
        return -1;
    }
    printf("sem.init ok\n");

    // 等待信信号挂出，等待成功后，将持有锁。
    if (!sem.P()) {
        printf("sem.wait failed.\n");
        return -1;
    }

    printf("sem.wait ok\n");

    sleep(20);  // 在 sleep 的过程中，再次运行该程序（新的程序实例），将等待锁。

    // 挂出信号灯，释放锁。
    if (!sem.V()) {
        printf("sem.post failed.\n");
        return -1;
    }

    printf("sem.post ok\n");

    // 销毁信号灯。
    if (!sem.destroy()) {
        printf("sem.destroy failed.\n");
        return -1;
    }
    printf("sem.destroy ok\n");
}