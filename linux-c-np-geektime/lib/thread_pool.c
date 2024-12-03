#include  <assert.h>
#include "thread_pool.h"
#include "utils.h"

struct thread_pool *thread_pool_new(struct event_loop *main_loop, int thread_number) {
    struct thread_pool *thread_pool = malloc(sizeof(struct thread_pool));
    thread_pool->main_loop = main_loop;
    thread_pool->position = 0;
    thread_pool->thread_number = thread_number;
    thread_pool->started = false;
    thread_pool->threads = NULL;
    return thread_pool;
}

void thread_pool_start(struct thread_pool *thread_pool) {
    assert(!thread_pool->started);
    assert_in_same_thread(thread_pool->main_loop);

    thread_pool->started = true;
    if (thread_pool->thread_number <= 0) {
        return;
    }

    //初始化并启动所有子线程
    thread_pool->threads = malloc(sizeof(struct event_loop_thread) * thread_pool->thread_number);
    for (int i = 0; i < thread_pool->thread_number; ++i) {
        event_loop_thread_init(&thread_pool->threads[i], i);
        event_loop_thread_start(&thread_pool->threads[i]);
    }
}

struct event_loop *thread_pool_peek_work_loop(struct thread_pool *thread_pool) {
    assert(thread_pool->started);
    assert_in_same_thread(thread_pool->main_loop);

    //默认选择主线程
    struct event_loop *return_loop = thread_pool->main_loop;

    //有子线程则选择一个子线程
    if (thread_pool->thread_number > 0) {
        return_loop = thread_pool->threads[thread_pool->position].loop;
        if (++thread_pool->position >= thread_pool->thread_number) {
            thread_pool->position = 0;
        }
    }

    return return_loop;
}