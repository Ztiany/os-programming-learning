#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "tcp_connection.h"

int handle_read(void *data) {

}

int handle_write(void *data) {

}

struct tcp_connection *tcp_connection_new(
        int connection_fd,
        struct event_loop *loop,
        connection_completed_call_back on_new_connection,
        message_call_back on_new_message,
        write_completed_call_back on_write_completed,
        connection_closed_call_back on_connection_closed
) {

    //创建 tcp_connection
    struct tcp_connection *connection = malloc(sizeof(struct tcp_connection));

    //设置循环器
    connection->loop = loop;

    //回调
    connection->on_new_connection = on_new_connection;
    connection->on_new_message = on_new_message;
    connection->on_write_completed = on_write_completed;
    connection->on_connection_closed = on_connection_closed;

    //每个连接都有自己的缓冲区
    connection->input_buffer = buffer_new();
    connection->output_buffer = buffer_new();

    //连接名称
    char *name = malloc(16);
    sprintf(name, "connection-%d\0", connection_fd);
    connection->name = name;

    //默认注册一个可读事件
    struct channel *channel = channel_new(connection_fd, EVENT_READ, handle_read, handle_write, connection);
    connection->channel = channel;

    //通知连接创建完成
    if (connection->on_new_connection) {
        connection->on_new_connection(connection);
    }

    //将通道设置给 event_loop，以便注册给 dispatcher
    event_loop_add_channel_event(loop, connection_fd, channel);

    //返回
    return connection;
}

int tcp_connection_send_data(
        struct tcp_connection *connection,
        void *data,
        int size
) {
    int size_written = 0;
    int size_left = size;

    struct channel *channel = connection->channel;
    struct buffer *output_buffer = connection->output_buffer;

    //先尝试直接发数据【条件：1 没有开启 write，也就是没有注册写事件；2 output_buffer 里面没有可读数据】
    if (!channel_event_is_writeable(channel) && buffer_readable_size(output_buffer) == 0) {
        size_written = write(channel->fd, data, size);
        if (size_written >= 0) {

        } else {

        }
    }

    return size_written;
}

int tcp_connection_send_buffer(
        struct tcp_connection *connection,
        struct buffer *buffer
) {

}

void tcp_connection_shutdown(struct tcp_connection *connection) {

}