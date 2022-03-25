#include "stdio.h"
#include "stdlib.h"
#include "sys/socket.h"
#include <netinet/in.h>
#include <obstack.h>
#include <unistd.h>
#include "../lib/log.h"

void read_data(int);


int main(int argc, char **argv) {
    //server 和 client fd
    int server_fd, client_fd;
    socklen_t client_len;
    //server 和 client 地址
    struct sockaddr_in server_addr, client_addr;

    //创建 server socket
    server_fd = socket(PF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    yolanda_msgx("socket success.");

    //设置服务器地址
    bzero(&server_addr, sizeof(server_addr));//erase the data of server_addr.
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(12345);

    //绑定到本地地址
    if (bind(server_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }
    yolanda_msgx("bind success.");


    //开始监听，等待队列设置为 1024
    if (listen(server_fd, 1024) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    yolanda_msgx("listen success.");

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
    /* 循环处理用户请求 */
    for (;;) {
        client_fd = accept(server_fd, (struct sockaddr *) &client_addr, &client_len);
        read_data(client_fd);
        /* 关闭连接套接字，注意不是监听套接字*/
        close(client_fd);
    }
#pragma clang diagnostic pop
}

void read_data(int client_fd) {

}