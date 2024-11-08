#include <pthread.h>
#include "utils.h"
#include "log.h"

void assert_in_same_thread(struct event_loop *loop) {
    if (loop->owner_thread_id != pthread_self()) {
        yolanda_errorx("loop was bound with thread(%s), but not running in it. exit program.", loop->thread_name);
        exit(EXIT_FAILURE);
    }
}

bool is_in_same_thread(struct event_loop *loop) {
    return loop->owner_thread_id == pthread_self();
}