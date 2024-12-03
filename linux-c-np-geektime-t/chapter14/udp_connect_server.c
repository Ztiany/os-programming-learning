#include <stdlib.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "../lib/log.h"
#include "../lib/common.h"

static int count;

/** 信号处理函数 */
static void recvfrom_int(int sig_no) {
    printf("\nreceived %d datagrams\n", count);
    exit(0);
}

int main(int argc, char **argv) {
    // 远程调试时，写入到 stdout 和 stderr 中的数据可能会保存在缓冲区中，导致客户端没有输出，通过下面的宏定义，可以让缓冲区不工作。
    NO_BUFFER(stdout)
    NO_BUFFER(stderr)

    //创建本地 socket
    int socket_fd;
    socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd < 0) {
        error(1, errno, "socket");
    }
    yolanda_debugx("socket_fd: %d", socket_fd);

    //准备本地地址，INADDR_ANY 相当于监听本机所有网卡地址
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERV_PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    //将 socket 绑定到本地地址。
    if (bind(socket_fd, TO_SOCK_ADDR(server_addr), sizeof(server_addr)) < 0) {
        error(1, errno, "bind");
    }
    yolanda_debugx("bind success");

    //处理 SIGINT 信号：键盘中断信号（Ctrl+C）
    signal(SIGINT, recvfrom_int);

    //客户端 socket 地址
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    //收消息的缓冲区
    char receive_buffer[BUFFER_SIZE];
    bzero(receive_buffer, sizeof(receive_buffer));
    //发送消息缓冲区的长度
    int send_buffer_size = sizeof("Hi, ") + BUFFER_SIZE;
    char send_buffer[send_buffer_size];
    bzero(send_buffer, send_buffer_size);

    //收消息计数
    count = 0;

    //先收消息，然后执行连接。
    int read_size = recvfrom(socket_fd, receive_buffer, BUFFER_SIZE, 0, TO_SOCK_ADDR(client_addr), &client_addr_len);
    if (read_size < 0) {
        error(1, errno, "recvfrom failed");
    }
    receive_buffer[BUFFER_SIZE - 1] = '\0';//设置字符串结束标识，防止可能的打印错误。
    yolanda_debugx("received message: %s", receive_buffer);

    //调用 connect 操作，将 UDP 套接字和客户端 client_addr 进行绑定
    if (connect(socket_fd, TO_SOCK_ADDR(client_addr), sizeof(client_addr)) < 0) {
        error(1, errno, "connect client failed");
    }
    yolanda_debugx("connect client success");


    //-----------------------------------------------------------------------------------
    // 连接客户端后，就进入了一对一的服务模式，将不再接收其他客户端的消息。
    //-----------------------------------------------------------------------------------
    yolanda_debugx("start to receive message.");
    while (strncmp(receive_buffer, "goodbye", 7) != 0) {
        //将接收的数据发回去
        bzero(send_buffer, send_buffer_size);
        sprintf(send_buffer, "Hi, %s", receive_buffer);
        if (send(socket_fd, send_buffer, strlen(send_buffer), 0) < 0) {
            error(1, errno, "send failed");
        }

        //接收数据
        bzero(receive_buffer, sizeof(receive_buffer));
        read_size = recv(socket_fd, receive_buffer, BUFFER_SIZE, 0);
        if (read_size < 0) {
            error(1, errno, "recvfrom failed");
        }
        receive_buffer[BUFFER_SIZE - 1] = '\0';//设置字符串结束标识，防止可能的打印错误。
        printf("received %d bytes: %s\n", read_size, receive_buffer);

        //计数
        count++;
    }

}