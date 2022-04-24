#include "stdio.h"
#include "stdlib.h"
#include "sys/socket.h"
#include <netinet/in.h>
#include <obstack.h>
#include <signal.h>
#include <stdbool.h>
#include "../lib/common.h"
#include "../lib/log.h"
#include "../lib/read.h"

static int count;

/** 信号处理函数 */
static void sig_int(int sig_no) {
    printf("\nreceived %d datagrams\n", count);
    exit(0);
}

int main(int argc, char **argv) {
    NO_BUFFER(stdout)
    NO_BUFFER(stderr)

    // 创建套接字
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        error(1, errno, "socket");
    }

    // 设置地址可重用
    int on = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    // 创建套接字地址
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERV_PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // 绑定到本地
    if (bind(listen_fd, TO_SOCK_ADDR(server_addr), sizeof(server_addr)) < 0) {
        error(1, errno, "bind");
    }

    // 开始监听
    if (listen(listen_fd, LISTEN_Q) < 0) {
        error(1, errno, "listen");
    }

    // 服务器端程序有一点需要注意，那就是对 SIGPIPE 这个信号的处理。
    signal(SIGINT, sig_int);
    signal(SIGPIPE, SIG_IGN);// SIG_IGN 表示忽略信号

    // 接收客户端连接
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int conn_fd = accept(listen_fd, TO_SOCK_ADDR(client_addr), &client_addr_len);
    if (conn_fd < 0) {
        error(1, errno, "accept");
    }

    //收消息的缓冲区
    char receive_buffer[BUFFER_SIZE + 1];

    //收消息计数
    count = 0;
    while (true) {
        int n = read_message(conn_fd, receive_buffer, BUFFER_SIZE);
        if (n < 0) {
            error(1, errno, "read_message");
        } else if (n == 0) {
            error(1, 0, "client closed");
        }
        receive_buffer[n] = '\0';
        yolanda_msgx("received: %s", receive_buffer);
        count++;
    }
}