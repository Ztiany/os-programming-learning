#include "stdio.h"
#include "stdlib.h"
#include "sys/socket.h"
#include <netinet/in.h>
#include <obstack.h>
#include <unistd.h>
#include "../lib/read.h"
#include "../lib/log.h"

void read_data(int);

int main(int argc, char **argv) {
    //创建 server socket
    int server_fd = socket(PF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    yolanda_debugx("socket success.");

    //设置服务器地址
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));//erase the data of server_addr.
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(12345);

    //绑定 socket 到本地地址
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
    yolanda_msgx("server started success and listening on 12345.");

    /* 循环处理用户请求 */
    for (;;) {
        //客户端地址
        struct sockaddr_in client_addr;
        socklen_t client_len;
        //接收客户端连接
        int client_fd = accept(server_fd, (struct sockaddr *) &client_addr, &client_len);
        //接收客户端数据
        read_data(client_fd);
        // 关闭连接套接字，注意不是监听套接字
        close(client_fd);
    }
}

void read_data(int client_fd) {
    size_t read_size;
    char buf[1024];
    int time = 0;

    for (;;) {
        fprintf(stdout, "block in read\n");

        read_size = readn(client_fd, buf, 1024);//readn 尝试从 fd 读取指定 size 的数据
        if (read_size == 0) {
            return;
        }

        time++;
        fprintf(stdout, "1K read for %d \n", time);

        //休眠 1 秒，用来模拟服务器端处理时延。
        usleep(1000);
    }
}