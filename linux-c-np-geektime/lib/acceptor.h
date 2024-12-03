/*
 ============================================================================

 Author      : Ztiany
 Description : NIO + 多路复用服务器：TCP Server Factory.

 ============================================================================
 */

#ifndef GEEKTIME_ACCEPTOR_H
#define GEEKTIME_ACCEPTOR_H

#include <malloc.h>
#include "common.h"

struct acceptor {
    int server_fd;
    int server_port;
};

/** 创建一个 tcp-server，并封装为 acceptor 返回。*/
struct acceptor *acceptor_init(int port);

/** 释放 acceptor。*/
void acceptor_free(struct acceptor *acceptor);

#endif //GEEKTIME_ACCEPTOR_H
