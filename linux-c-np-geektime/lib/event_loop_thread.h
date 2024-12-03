#ifndef GEEKTIME_EVENT_LOOP_THREAD_H
#define GEEKTIME_EVENT_LOOP_THREAD_H

#include "event_loop.h"

/** 运行 event_loop 的线程 */
struct event_loop_thread {
    struct event_loop *loop;
    pthread_t thread_id;
    pthread_mutex_t mutex;
    pthread_cond_t condition;
    const char *thread_name;

    /** connections handled【没有用到】 */
    long thread_count;
};

/** 初始化一个 event loop 线程 */
int event_loop_thread_init(struct event_loop_thread *thread, int counter);

/** 让子线程开始运行 event_loop【必须由主线程调用】 */
struct event_loop *event_loop_thread_start(struct event_loop_thread *thread);

#endif //GEEKTIME_EVENT_LOOP_THREAD_H
