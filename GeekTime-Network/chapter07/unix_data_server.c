#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdbool.h>
#include "../lib/common.h"
#include "../lib/log.h"

/*请设置运行参数为 /tmp/unix_data_socket_test */
int main(int argc, char **argv) {
    NO_BUFFER(stdout)
    NO_BUFFER(stderr)

    if (argc != 2) {
        error(1, 0, "usage: unix-data-server <local_path>");
    }

    //创建 Socket
    int socket_fd = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if (socket_fd < 0) {
        error(1, errno, "socket() failed");
    }

    //如果有之前的文件，则先删除
    const char *socket_path = argv[1];
    unlink(socket_path);

    //将 socket 绑定到本地地址
    struct sockaddr_un socket_addr;
    bzero(&socket_addr, sizeof(socket_addr));
    socket_addr.sun_family = AF_LOCAL;
    strcpy(socket_addr.sun_path, socket_path);
    if (bind(socket_fd, TO_SOCK_ADDR(socket_addr), sizeof(socket_addr)) < 0) {
        error(1, errno, "bind failed");
    }
    yolanda_msgx("server data socket is bound to %s", socket_path);

    //接收缓冲区
    char receive_buffer[BUFFER_SIZE];

    //发送缓冲区
    size_t send_buffer_size = sizeof("Hi, ") + BUFFER_SIZE;
    char send_buffer[send_buffer_size];

    //客户端 socket 地址
    struct sockaddr_un client_addr;
    socklen_t client_addr_len = sizeof(client_addr);//必须要设置为实际长度
    bzero(&client_addr, sizeof(client_addr));

    //开始收发数据
    while (true) {
        //接收数据
        bzero(receive_buffer, sizeof(receive_buffer));
        int received_size = recvfrom(socket_fd,
                                     receive_buffer,
                                     sizeof(receive_buffer),
                                     0,
                                     TO_SOCK_ADDR(client_addr),
                                     &client_addr_len
        );
        if (received_size == 0) {
            yolanda_msgx("received 0 byte. break.");
            break;
        }
        yolanda_msgx("received %s from %s", receive_buffer, client_addr.sun_path);

        //回写数据
        bzero(send_buffer, sizeof(send_buffer));
        sprintf(send_buffer, "Hi, %s", receive_buffer);
        size_t send_bytes = strlen(send_buffer);//真正要发送的长度
        yolanda_msgx("try send %s to %s", send_buffer, client_addr.sun_path);
        if (sendto(socket_fd, send_buffer, send_bytes, 0, TO_SOCK_ADDR(client_addr), client_addr_len) != send_bytes) {
            error(1, errno, "sendto error");
        }
    }//end while

    close(socket_fd);
    exit(EXIT_SUCCESS);
}