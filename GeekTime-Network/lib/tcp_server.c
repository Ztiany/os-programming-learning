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