#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include "../lib/common.h"
#include "../lib/log.h"

/*请设置运行参数为 /tmp/unix_data_socket_test */
int main(int argc, char **argv) {
    NO_BUFFER(stdout)
    NO_BUFFER(stderr)

    if (argc != 2) {
        error(1, 0, "usage: unix-data-client <local_path>");
    }

    //创建 Socket
    int socket_fd = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if (socket_fd < 0) {
        error(1, errno, "socket() failed");
    }

    //本地 socket 地址，对应的文件路径通过 tmpnam 生成。
    struct sockaddr_un socket_addr;
    bzero(&socket_addr, sizeof(socket_addr));
    socket_addr.sun_family = AF_LOCAL;
    strcpy(socket_addr.sun_path, tmpnam(NULL));

    //绑定 socket【不同于 UDP，Client Unix Data Gram Socket 也需要 bind 到本地路径。】
    if (bind(socket_fd, TO_SOCK_ADDR(socket_addr), sizeof(socket_addr)) < 0) {
        error(1, errno, "bind failed");
    }
    yolanda_msgx("client data socket is bound to %s", socket_addr.sun_path);

    //服务端 socket 地址
    struct sockaddr_un server_addr;
    bzero(&socket_addr, sizeof(socket_addr));
    server_addr.sun_family = AF_LOCAL;
    strcpy(server_addr.sun_path, argv[1]);

    //准备发送缓冲区
    char send_buffer[BUFFER_SIZE];
    bzero(send_buffer, sizeof(send_buffer));

    //准备接收缓冲区
    char receive_buffer[BUFFER_SIZE];
    bzero(receive_buffer, sizeof(receive_buffer));

    //开始收发数据
    while (fgets(send_buffer, BUFFER_SIZE, stdin) != NULL) {
        //准备数据【从控制台输入】
        size_t real_len = strlen(send_buffer);
        if (send_buffer[real_len - 1] == '\n') {
            send_buffer[real_len - 1] = 0;
        }

        //发送
        yolanda_msgx("try send %s to %s", send_buffer, argv[1]);
        if (sendto(socket_fd, send_buffer, real_len, 0, TO_SOCK_ADDR(server_addr), sizeof(server_addr)) != real_len) {
            error(1, errno, "sendto error");
        }

        //接收数据
        int received_size = recvfrom(socket_fd, receive_buffer, sizeof(receive_buffer), 0, NULL, NULL);
        yolanda_msgx("received size = %d, content = %s", received_size, receive_buffer);

        //清空缓冲区
        bzero(send_buffer, sizeof(send_buffer));
        bzero(receive_buffer, sizeof(receive_buffer));
    }

    close(socket_fd);
    exit(EXIT_SUCCESS);
}