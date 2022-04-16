#include "stdio.h"
#include "stdlib.h"
#include "sys/socket.h"
#include <netinet/in.h>
#include <obstack.h>
#include <unistd.h>
#include <signal.h>
#include "../lib/common.h"
#include "../lib/log.h"
#include "message_protocol.h"

static int count;

/** 信号处理函数 */
static void sig_int(int sig_no) {
    printf("\nreceived %d datagrams\n", count);
    exit(0);
}

/* 请设置运行参数为一个数字，比如 5，则表示服务器收到 ping 后，睡眠 5 秒。*/
int main(int argc, char **argv) {
    if (argc != 2) {
        error(1, 0, "usage: 12-ping-server <sleeping_time>");
    }
    //服务器睡眠时间，模拟时延
    int sleeping_time = atoi(argv[1]);

    // 创建套接字
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        error(1, errno, "socket");
    }

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

    // 收消息计数
    count = 0;
    // 消息
    messageObject message;

    for (;;) {
        //读数据
        bzero(&message, sizeof(message));
        size_t size_read = read(conn_fd, (char *) &message, MAX_LINE);
        if (size_read < 0) {
            error(1, errno, "read error");
        } else if (size_read == 0) {
            error(1, 0, "client closed.");
        }
        printf("received %zu bytes\n", size_read);
        count++;

        switch (ntohl(message.type)) {
            case MSG_PING:
                printf("received ping\n");
                messageObject pong_message;
                pong_message.type = htonl(MSG_PONG);
                sleep(sleeping_time);
                ssize_t rc = send(conn_fd, (char *) &pong_message, sizeof(pong_message), 0);
                if (rc < 0) {
                    error(1, errno, "send failure");
                }
                break;
            case MSG_TYPE1:
                printf("process  MSG_TYPE1 \n");
                break;
            case MSG_TYPE2:
                printf("process  MSG_TYPE2 \n");
                break;
            default:
                error(1, 0, "unknown message type");
        }
    }

}