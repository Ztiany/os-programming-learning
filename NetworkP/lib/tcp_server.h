/*
 ============================================================================
 
 Author      : Ztiany
 Description : TCP Server Factory.

 ============================================================================
 */

#ifndef GEEKTIME_TCP_SERVER_H
#define GEEKTIME_TCP_SERVER_H

#include "event_loop.h"
#include "acceptor.h"
#include "thread_pool.h"
#include "tcp_server.h"
#include "buffer.h"

/** 创建一个 TCP Server，并接收一个 TCP Client，返回 Client 的 fd。*/
int tcp_server_accept_one(int port);

/** 创建一个 TCP Server，并开始监听，返回 Server 的 fd。*/
int tcp_server_listen(int port);

/** 创建一个非阻塞 TCP Server，并开始监听，返回 Server 的 fd。*/
int tcp_server_listen_non_blocking(int port);


//=========================================================
// TCP Server 封装
//=========================================================

/* 前置声明 */
struct tcp_connection;

/** tcp 连接建立时的回调 */
typedef int (*connection_completed_call_back)(struct tcp_connection *connection);

/** tcp 消息回调 */
typedef int(*message_call_back)(struct buffer *buffer, struct tcp_connection *connection);

/** tcp 写完成回调 */
typedef int(*write_completed_call_back)(struct tcp_connection *connection);

/** tcp 连接关闭回调 */
typedef int(*connection_closed_call_back)(struct tcp_connection *connection);

/** 表示一个 TCP 服务端所需要的所有信息 */
struct tcp_server {
    /** 监听的端口 */
    int port;

    /** 处理该 server 的循环器*/
    struct event_loop *loop;

    struct acceptor *acceptor;

    /* tcp 回调 */
    connection_completed_call_back on_new_connection;
    message_call_back on_new_message;
    write_completed_call_back on_write_completed;
    connection_closed_call_back on_connection_closed;

    /** 处理 tcp read/write 的线程数 */
    int io_thread_number;
    struct thread_pool *thread_pool;

    /** for callback use: http_server */
    void *data;
};

/** 创建一个 tcp_server */
struct tcp_server *tcp_server_new(
        struct event_loop *loop,
        struct acceptor *acceptor,
        connection_completed_call_back on_new_connection,
        message_call_back on_new_message,
        write_completed_call_back on_write_completed,
        connection_closed_call_back on_connection_closed,
        int io_thread_number
);

/** 启动该 tcp server */
void tcp_server_start(struct tcp_server *server);

/** 设置回调数据 */
void tcp_server_set_callback_data(struct tcp_server *server, void *data);

#endif //GEEKTIME_TCP_SERVER_H
