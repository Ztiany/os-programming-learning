#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdbool.h>
#include "../lib/common.h"
#include "../lib/log.h"

/*请设置运行参数为 /tmp/unix_stream_socket_test */
int main(int argc, char **argv) {
    NO_BUFFER(stdout)
    NO_BUFFER(stderr)

    if (argc != 2) {
        error(1, 0, "usage: unix-stream-server <local_path>");
    }

    //创建 Socket
    int listen_fd = socket(AF_LOCAL, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        error(1, errno, "socket() failed");
    }

    //如果有之前的文件，则先删除
    const char *socket_path = argv[1];
    unlink(socket_path);

    //将 socket 绑定到本地地址
    struct sockaddr_un listen_addr;
    bzero(&listen_addr, sizeof(listen_addr));
    listen_addr.sun_family = AF_LOCAL;
    strcpy(listen_addr.sun_path, socket_path);
    if (bind(listen_fd, TO_SOCK_ADDR(listen_addr), sizeof(listen_addr)) < 0) {
        error(1, errno, "bind failed");
    }

    //开始监听
    if (listen(listen_fd, LISTEN_Q) < 0) {
        error(1, errno, "listen failed");
    }
    yolanda_msgx("server is now listening %s", socket_path);

    //开始接收连接
    int client_fd;
    struct sockaddr_un client_addr;
    socklen_t client_addr_len;
    while ((client_fd = accept(listen_fd, TO_SOCK_ADDR(client_addr), &client_addr_len)) < 0) {
        if (errno == EINTR)
            continue;
        else
            error(1, errno, "accept failed");
    }
    yolanda_msgx("server accepted a client %d", client_fd);

    //接送缓冲区
    char receive_buffer[BUFFER_SIZE];

    //发送缓冲区
    size_t send_buffer_size = sizeof("Hi, ") + BUFFER_SIZE;
    char send_buffer[send_buffer_size];

    //开始收发数据
    while (true) {
        //接收数据
        bzero(receive_buffer, sizeof(receive_buffer));
        if (read(client_fd, receive_buffer, BUFFER_SIZE) == 0) {
            yolanda_msgx("read nothing. client %d exit.", client_fd);
            break;
        }
        yolanda_msgx("received %s from client %d", receive_buffer, client_fd);

        //回写数据
        bzero(send_buffer, sizeof(send_buffer));
        sprintf(send_buffer, "Hi, %s", receive_buffer);
        size_t send_bytes = strlen(send_buffer);//真正要发送的长度
        yolanda_msgx("try send %s to client %d", send_buffer, client_fd);
        if (write(client_fd, send_buffer, send_bytes) != send_bytes) {
            error(1, errno, "write error");
        }
    }//end while

    close(client_fd);
    close(listen_fd);

    exit(EXIT_SUCCESS);
}