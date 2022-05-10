/*
 ============================================================================

 Author      : Ztiany
 Description : 基于 poll + 非阻塞 I/O 的多线程主从 reactor 模型的 tcp server

 ============================================================================
 */

#include "../lib/tcp_server.h"
#include "../lib/log.h"
#include "../lib/chars.h"
#include "../lib/tcp_connection.h"

int on_new_connection(struct tcp_connection *connection) {
    yolanda_msgx("new connection.");
    return 1;
}

int on_new_message(struct buffer *input, struct tcp_connection *connection) {
    yolanda_msgx("new message: %s", input->data);

    struct buffer *output = buffer_new();
    int size = buffer_readable_size(input);
    for (int i = 0; i < size; i++) {
        buffer_append_char(output, rot13_char(buffer_read_char(input)));
    }
    tcp_connection_send_buffer(connection, output);

    return 1;
}

int on_write_completed(struct tcp_connection *connection) {
    yolanda_msgx("write completed.");
    return 1;
}

int on_connection_closed(struct tcp_connection *connection) {
    yolanda_msgx("connection closed.");
    return 1;
}

int main(int argc, char **argv) {
    NO_BUFFER(stdout)
    NO_BUFFER(stderr)

    //主线程的 event_loop
    struct event_loop *main_loop = event_loop_init();
    yolanda_msgx("main_loop init completed.");

    //初始化 acceptor
    struct acceptor *acceptor = acceptor_init(SERV_PORT);
    yolanda_msgx("acceptor init completed.");


    //创建 tcp 服务器
    struct tcp_server *server = tcp_server_new(
            main_loop,
            acceptor,
            on_new_connection,
            on_new_message,
            on_write_completed,
            on_connection_closed,
            4
    );
    yolanda_msgx("tcp_server init completed.");

    // 启动 server
    tcp_server_start(server);
    yolanda_msgx("tcp_server start completed.");

    // 启动事件循环器
    event_loop_run(main_loop);
    yolanda_msgx("main_loop run completed.");
}