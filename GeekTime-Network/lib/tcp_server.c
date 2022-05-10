#include "sys/socket.h"
#include <netinet/in.h>
#include <obstack.h>
#include <signal.h>
#include <fcntl.h>
#include "common.h"
#include "log.h"
#include "tcp_server.h"
#include "tcp_connection.h"

int tcp_server_accept_one(int port) {
    // 创建套接字
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        error(1, errno, "tcp_server socket() failed");
    }

    // 设置地址可重用
    int on = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    // 创建套接字地址
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // 绑定到本地
    if (bind(server_fd, TO_SOCK_ADDR(server_addr), sizeof(server_addr)) < 0) {
        error(1, errno, "tcp_server bind() failed");
    }

    // 开始监听
    if (listen(server_fd, LISTEN_Q) < 0) {
        error(1, errno, "tcp_server listen() failed");
    }

    signal(SIGPIPE, SIG_IGN);// SIG_IGN 表示忽略信号

    // 接受客户端连接
    struct sockaddr_in client_addr;
    bzero(&client_addr, sizeof(client_addr));
    socklen_t client_addr_len = sizeof(client_addr);
    int client_fd = accept(server_fd, TO_SOCK_ADDR(client_addr), &client_addr_len);

    if (client_fd < 0) {
        error(1, errno, "tcp_server accept() failed");
    }

    return client_fd;
}

int tcp_server_listen(int port) {
    // 创建套接字
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        error(1, errno, "tcp_server socket() failed");
    }

    // 设置地址可重用
    int on = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    // 创建套接字地址
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // 绑定到本地
    if (bind(server_fd, TO_SOCK_ADDR(server_addr), sizeof(server_addr)) < 0) {
        error(1, errno, "tcp_server bind() failed");
    }

    // 开始监听
    if (listen(server_fd, LISTEN_Q) < 0) {
        error(1, errno, "tcp_server listen() failed");
    }

    signal(SIGPIPE, SIG_IGN);// SIG_IGN 表示忽略信号

    return server_fd;
}

int tcp_server_listen_non_blocking(int port) {

// 创建套接字
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        error(1, errno, "tcp_server socket() failed");
    }

    //设置为非阻塞
    make_nonblocking(server_fd);

    // 设置地址可重用
    int on = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    // 创建套接字地址
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // 绑定到本地
    if (bind(server_fd, TO_SOCK_ADDR(server_addr), sizeof(server_addr)) < 0) {
        error(1, errno, "tcp_server bind() failed");
    }

    // 开始监听
    if (listen(server_fd, LISTEN_Q) < 0) {
        error(1, errno, "tcp_server listen() failed");
    }

    signal(SIGPIPE, SIG_IGN);// SIG_IGN 表示忽略信号

    return server_fd;
}

void make_nonblocking(int fd) {
    fcntl(fd, F_SETFL, O_NONBLOCK);
}

//=========================================================
// TCP Server 封装
//=========================================================

/** 创建一个 tcp_server */
struct tcp_server *tcp_server_new(
        struct event_loop *loop,
        struct acceptor *acceptor,
        connection_completed_call_back on_new_connection,
        message_call_back on_new_message,
        write_completed_call_back on_write_completed,
        connection_closed_call_back on_connection_closed,
        int io_thread_number
) {
    struct tcp_server *server = malloc(sizeof(struct tcp_server));

    //循环器
    server->loop = loop;
    server->acceptor = acceptor;
    server->port = acceptor->server_port;

    //回调
    server->on_new_connection = on_new_connection;
    server->on_new_message = on_new_message;
    server->on_write_completed = on_write_completed;
    server->on_connection_closed = on_connection_closed;

    //线程
    server->io_thread_number = io_thread_number;
    server->thread_pool = thread_pool_new(loop, server->io_thread_number);

    //附带数据
    server->data = NULL;

    return server;
}

int handle_connection_established(void *data) {
    struct tcp_server *server = (struct tcp_server *) data;
    struct acceptor *acceptor = server->acceptor;

    // 接收连接
    struct sockaddr_in client_addr;
    bzero(&client_addr, sizeof(client_addr));
    socklen_t client_addr_len = sizeof(client_addr);

    int client_fd = accept(acceptor->server_fd, TO_SOCK_ADDR(client_addr), &client_addr_len);
    make_nonblocking(client_fd);

    yolanda_msgx("A new client(%d) has been accepted.", client_fd);

    // 选择一个 event_loop 来服务该连接
    struct event_loop *service_loop = thread_pool_peek_work_loop(server->thread_pool);

    yolanda_msgx("select thread(%s) to server client(%d).", service_loop->thread_name, client_fd);

    // 创建一个 tcp_connection 来代表该连接
    struct tcp_connection *connection = tcp_connection_new(
            client_fd,
            service_loop,
            server->on_new_connection,
            server->on_new_message,
            server->on_write_completed,
            server->on_connection_closed
    );

    //for callback use
    if (server->data != NULL) {
        connection->data = server->data;
    }

    return 1;
}

/** 启动该 tcp server */
void tcp_server_start(struct tcp_server *server) {
    struct acceptor *acceptor = server->acceptor;
    struct event_loop *loop = server->loop;

    // 启动服务 I/O 的线程池
    thread_pool_start(server->thread_pool);
    yolanda_msgx("thread pool start completed");

    // 创建一个通道，处理新连接到达
    struct channel *acceptor_channel = channel_new(
            acceptor->server_fd,
            EVENT_READ,
            handle_connection_established,
            NULL,
            server
    );

    // 注册该通道
    event_loop_add_channel_event(loop, acceptor->server_fd, acceptor_channel);
}

/** 设置回调数据 */
void tcp_server_set_callback_data(struct tcp_server *server, void *data) {
    if (data != NULL) {
        server->data = data;
    }
}
