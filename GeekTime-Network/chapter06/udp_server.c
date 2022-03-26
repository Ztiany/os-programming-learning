#include <stdlib.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "../lib/log.h"
#include "../lib/common.h"

static int count;

static void recvfrom_int(int sig_no) {
    printf("\nreceived %d datagrams\n", count);
    exit(0);
}

int main(int argc, char **argv) {
    int socket_fd;
    struct sockaddr_in server_addr;

    //创建 socket
    socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd < 0) {
        error(1, errno, "socket");
    }

    //准备本地地址
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERV_PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    //绑定到本地地址
    if (bind(socket_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        error(1, errno, "bind");
    }

    //处理 SIGINT 信号：键盘中断信号（Ctrl+C）
    signal(SIGINT, recvfrom_int);

    //客户端地址
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    //收消息的缓冲区
    char message[MAX_LINE];
    //发送消息缓冲区的长度
    int send_size = sizeof("Hi, ") + MAX_LINE;
    //收消息计数
    count = 0;

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
    for (;;) {
        int read_size = recvfrom(socket_fd, message, MAX_LINE, 0, (struct sockaddr *) &client_addr, &client_addr_len);
        message[read_size] = 0;//最后一位置为 0，字符串结束标识。
        printf("received %d bytes: %s\n", read_size, message);

        char send_line[send_size];
        sprintf(send_line, "Hi, %s", message);
        sendto(socket_fd, send_line, strlen(send_line), 0, (struct sockaddr *) &client_addr, client_addr_len);

        count++;
    }
#pragma clang diagnostic pop

}