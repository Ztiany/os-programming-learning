#include "stdio.h"
#include "stdlib.h"
#include "sys/socket.h"
#include <netinet/in.h>
#include <obstack.h>
#include <unistd.h>
#include <signal.h>
#include "../lib/common.h"
#include "../lib/log.h"

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
    char receive_buffer[BUFFER_SIZE];

    //发送消息缓冲区的长度
    int send_size = sizeof("Hi, ") + BUFFER_SIZE;
    char send_buffer[send_size];

    //收消息计数
    count = 0;
    for (;;) {
        //读数据
        bzero(receive_buffer, sizeof(receive_buffer));
        size_t size_read = read(conn_fd, receive_buffer, MAX_LINE);
        if (size_read < 0) {
            error(1, errno, "read error");
        } else if (size_read == 0) {
            error(1, 0, "client closed.");
        }
        yolanda_msgx("server received: %s", receive_buffer);
        count++;

        //返回数据
        bzero(send_buffer, sizeof(send_buffer));
        sprintf(send_buffer, "Hi, %s", receive_buffer);
        sleep(5); //注意，在发送之前，让服务器端程序休眠了 5 秒，以模拟服务器端处理的时间。
        size_t size_write = write(conn_fd, send_buffer, strlen(send_buffer));
        if (size_write < 0) {
            error(1, errno, "write error");
        }
    }

}