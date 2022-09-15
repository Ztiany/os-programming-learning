//
// Created by Alien on 2022/4/16.
//

#ifndef GEEKTIME_MESSAGE_PROTOCOL_H
#define GEEKTIME_MESSAGE_PROTOCOL_H

#include <stdlib.h>

typedef struct {
    u_int32_t type;
    char data[1024];
} messageObject;

#define MSG_PING          1
#define MSG_PONG          2
#define MSG_TYPE1        11
#define MSG_TYPE2        21

#endif //GEEKTIME_MESSAGE_PROTOCOL_H
