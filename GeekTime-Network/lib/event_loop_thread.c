#include <stdio.h>
#include <assert.h>
#include "event_loop_thread.h"
#include "log.h"

int event_loop_thread_init(struct event_loop_thread *thread, int counter) {
    pthread_mutex_init(&thread->mutex, NULL);
    pthread_cond_init(&thread->condition, NULL);

    thread->loop = NULL;
    thread->thread_count = 0;
    thread->thread_id = 0;

    char *buf = malloc(16);
    sprintf(buf, "S-Thread-%d\0", counter + 1);
    thread->thread_name = buf;

    return 1;
}

void *event_loop_thread_run(void *arg) {
    struct event_loop_thread *thread = (struct event_loop_thread *) arg;

    // 加锁
    pthread_mutex_lock(&thread->mutex);

    //初始化自己的 event_loop，然后通知
    struct event_loop *loop = event_loop_init_with_name(thread->thread_name);
    thread->loop = loop;
    pthread_cond_signal(&thread->condition);

    // 解锁
    pthread_mutex_unlock(&thread->mutex);

    //运行起来
    event_loop_run(loop);

    return NULL;
}

struct event_loop *event_loop_thread_start(struct event_loop_thread *thread) {
    //启动子线程，运行 event_loop_thread_run
    pthread_create(&thread->thread_id, NULL, event_loop_thread_run, thread);

    // 加锁
    assert(pthread_mutex_lock(&thread->mutex) == 0);

    // 等待子线程初始化完毕 event_loop
    while (thread->loop == NULL) {
        assert(pthread_cond_wait(&thread->condition, &thread->mutex) == 0);
    }

    // 解锁
    assert(pthread_mutex_unlock(&thread->mutex) == 0);

    yolanda_msgx("event loop thread(%s) has started", thread->thread_name);

    return thread->loop;
}