#ifndef GEEKTIME_THREAD_POOL_H
#define GEEKTIME_THREAD_POOL_H

#include "event_loop.h"
#include "event_loop_thread.h"

/** 线程池封装 */
struct thread_pool {
    /** 创建 thread pool 的主线程 */
    struct event_loop *main_loop;

    /** 是否已经启动 */
    bool started;

    /** 线程数量 */
    int thread_number;

    /** 线程池里面的所有线程 */
    struct event_loop_thread *threads;

    /** 算法支持：用于帮助计算下一次用哪个线程服务新的 connection */
    int position;
};

/** 创建一个线程池 */
struct thread_pool *thread_pool_new(struct event_loop *main_loop, int thread_number);

/** 启动线程池*/
void thread_pool_start(struct thread_pool *thread_pool);

/** 选择一个线程来进行服务【每个线程对应一个 event_loop，所以直接返回 event_loop】*/
struct event_loop *thread_pool_peek_work_loop(struct thread_pool *thread_pool);

#endif //GEEKTIME_THREAD_POOL_H
