#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include "tcp_connection.h"
#include "log.h"
#include "utils.h"

void handle_connection_closed(struct tcp_connection *connection) {
    struct event_loop *loop = connection->loop;
    struct channel *channel = connection->channel;
    event_loop_remove_channel_event(loop, channel->fd, channel);
    if (connection->on_connection_closed) {
        connection->on_connection_closed(connection);
    }
}

int handle_read(void *data) {
    //handle_write 回调在 tcp_connection_new 被设置，tcp_connection_new 中的 channel 的 data 就是 tcp_connection，因此可以转换为 tcp_connection。
    struct tcp_connection *connection = (struct tcp_connection *) data;
    struct channel *channel = connection->channel;
    struct buffer *input_buffer = connection->input_buffer;

    if (buffer_socket_read(input_buffer, channel->fd) > 0) {
        //框架将网络数据读取到 buffer 里，而应用程序读取 buffer 里的数据
        if (connection->on_new_message) {
            connection->on_new_message(input_buffer, connection);
        }
    } else {// 小于 0 表示出错，等于 0 表示对方已经关闭，都要关闭连接。
        handle_connection_closed(connection);
    }

    return 1;
}

int handle_write(void *data) {
    //handle_write 回调在 tcp_connection_new 被设置，tcp_connection_new 中的 channel 的 data 就是 tcp_connection，因此可以转换为 tcp_connection。
    struct tcp_connection *connection = (struct tcp_connection *) data;
    struct event_loop *loop = connection->loop;
    assert_in_same_thread(loop);

    struct buffer *output_buffer = connection->output_buffer;
    struct channel *channel = connection->channel;

    int size_written = write(
            channel->fd,
            output_buffer->data + output_buffer->read_index,
            buffer_readable_size(output_buffer)
    );

    if (size_written > 0) {
        //更新索引
        output_buffer->read_index += size_written;
        //如果数据完全发送出去，就不需要继续了，此时就可以将 write 禁用。
        if (buffer_readable_size(output_buffer) == 0) {
            //这里的调用要求 channel 的 data 必须为 event_loop，但事实上这里 channel 的 data 为 tcp_connection，为什么没有问题呢？因为 tcp_connection 结构体的第一个成员就是 event_loop。
            channel_event_disable_write(channel);
        }
        //通知写完成。TODO？为什么不是完全写完才通知呢？
        if (connection->on_write_completed) {
            connection->on_write_completed(connection);
        }
    } else {
        yolanda_msgx("handle_write for tcp connection %s", connection->name);
    }

    return 1;
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
    int fault = false;

    struct channel *channel = connection->channel;
    struct buffer *output_buffer = connection->output_buffer;

    //首先尝试直接发数据【条件：1 没有开启 write，也就是没有注册写事件；2 output_buffer 里面没有需要发送的数据】
    if (!channel_event_is_writeable(channel) && buffer_readable_size(output_buffer) == 0) {
        size_written = write(channel->fd, data, size);
        if (size_written >= 0) {
            size_left -= size_written;
        } else {
            size_written = 0;
            // EWOULDBLOCK 其实就是 EAGAIN，它们的解释是：
            // They are often raised when performing non-blocking I/O. It means "there is no data available right now, try again later".
            if (errno != EWOULDBLOCK) {
                if (errno == EPIPE || errno == ECONNRESET) {//connection reset.
                    fault = true;
                }
            }
        }
    }

    if (!fault && size_left > 0) {
        //拷贝到 Buffer 中，Buffer 的数据由框架接管。
        buffer_append(output_buffer, data + size_written, size_left);
        //开启 write
        if (!channel_event_is_writeable(channel)) {
            //这里的调用要求 channel 的 data 必须为 event_loop，但事实上这里 channel 的 data 为 tcp_connection，为什么没有问题呢？因为 tcp_connection 结构体的第一个成员就是 event_loop。
            channel_event_enable_write(channel);
        }
    } else {
        //TODO：通知错误或完成？
    }

    return size_written;
}

int tcp_connection_send_buffer(
        struct tcp_connection *connection,
        struct buffer *buffer
) {
    int size = buffer_readable_size(buffer);
    int result = tcp_connection_send_data(connection, buffer->data + buffer->read_index, size);
    buffer->read_index += result;
    return result;
}

void tcp_connection_shutdown(struct tcp_connection *connection) {
    if (shutdown(connection->channel->fd, SHUT_WR) < 0) {
        yolanda_msgx("tcp_connection_shutdown failed, socket == %d", connection->channel->fd);
    }
}