#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include "../lib/common.h"
#include "../lib/log.h"

/*请设置运行参数为 /tmp/unix_stream_socket_test */
int main(int argc, char **argv) {
    NO_BUFFER(stdout)
    NO_BUFFER(stderr)

    if (argc != 2) {
        error(1, 0, "usage: unix-stream-client <local_path>");
    }

    //创建 Socket
    int client_fd = socket(AF_LOCAL, SOCK_STREAM, 0);
    if (client_fd < 0) {
        error(1, errno, "socket() failed");
    }

    //连接指定的 socket
    struct sockaddr_un server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sun_family = AF_LOCAL;
    strcpy(server_addr.sun_path, argv[1]);
    if (connect(client_fd, TO_SOCK_ADDR(server_addr), sizeof(server_addr)) < 0) {
        error(1, errno, "connect failed");
    }

    //接收缓冲区
    char receive_buffer[BUFFER_SIZE];
    bzero(receive_buffer, sizeof(receive_buffer));

    //发送缓冲区
    char send_buffer[BUFFER_SIZE];
    bzero(send_buffer, sizeof(send_buffer));

    //开始收发数据
    while (fgets(send_buffer, BUFFER_SIZE, stdin) != NULL) {
        //准备数据【从控制台输入】
        size_t real_len = strlen(send_buffer);
        if (send_buffer[real_len - 1] == '\n') {
            send_buffer[real_len - 1] = 0;
        }

        //发送数据
        yolanda_msgx("try send: %s", send_buffer);
        if (write(client_fd, send_buffer, real_len) != real_len) {
            error(1, errno, "write error");
        }

        //接收数据
        if (read(client_fd, receive_buffer, BUFFER_SIZE) == 0) {
            error(1, errno, "server terminated prematurely");
        }
        yolanda_msgx("received: %s", receive_buffer);

        bzero(send_buffer, sizeof(send_buffer));
        bzero(receive_buffer, sizeof(receive_buffer));
    }

    close(client_fd);
    exit(EXIT_SUCCESS);
}