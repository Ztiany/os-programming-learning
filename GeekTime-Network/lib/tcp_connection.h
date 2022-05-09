#ifndef GEEKTIME_TCP_CONNECTION_H
#define GEEKTIME_TCP_CONNECTION_H

#include "event_loop.h"
#include "channel.h"
#include "buffer.h"
#include "tcp_server.h"

struct tcp_connection {
    struct event_loop *loop;
    struct channel *channel;
    const char *name;

    /** 接收缓冲区 */
    struct buffer *input_buffer;
    /** 发送缓冲区 */
    struct buffer *output_buffer;

    /* tcp 回调 */
    connection_completed_call_back on_new_connection;
    message_call_back on_new_message;
    write_completed_call_back on_write_completed;
    connection_closed_call_back on_connection_closed;

    /** for callback use: http_server */
    void *data;
    /** for callback use  */
    void *request;
    /** for callback use */
    void *response;
};

/** 创建一个 tcp_connection */
struct tcp_connection *tcp_connection_new(
        int connection_fd,
        struct event_loop *loop,
        connection_completed_call_back on_new_connection,
        message_call_back on_new_message,
        write_completed_call_back on_write_completed,
        connection_closed_call_back on_connection_closed
);

/** 发送数据 */
int tcp_connection_send_data(
        struct tcp_connection *connection,
        void *data,
        int size
);

/** 发送数据 */
int tcp_connection_send_buffer(
        struct tcp_connection *connection,
        struct buffer *buffer
);

/** 关闭 tcp 连接 */
void tcp_connection_shutdown(struct tcp_connection *connection);

#endif //GEEKTIME_TCP_CONNECTION_H
