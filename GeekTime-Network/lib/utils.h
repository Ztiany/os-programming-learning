#ifndef GEEKTIME_UTILS_H
#define GEEKTIME_UTILS_H

#include <stdlib.h>
#include "event_loop.h"

void assert_in_same_thread(struct event_loop *loop);

bool is_in_same_thread(struct event_loop *loop);

#endif //GEEKTIME_UTILS_H
