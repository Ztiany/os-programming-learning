#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../lib/log.h"
#include "../lib/common.h"

/** 启动参数为 127.0.0.1【内容与 udp_connect_client1 内容完全一致】，目的是为了启动多个客户端*/
int main(int argc, char **argv) {
    NO_BUFFER(stdout)
    NO_BUFFER(stderr)

    if (argc != 2) {
        error(1, 0, "usage: 14-udp-connect-client-2 <IPaddress>");
    }

    //创建套接字
    int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd == -1) {
        error(1, errno, "socket create failed");
    }

    //创建远程地址
    struct sockaddr_in remote_addr;
    bzero(&remote_addr, sizeof(remote_addr));
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_port = htons(SERV_PORT);
    inet_pton(AF_INET, argv[1], &remote_addr.sin_addr);

    //执行 UDP 连接
    if (connect(socket_fd, (struct sockaddr *) &remote_addr, sizeof(remote_addr)) == -1) {
        error(1, errno, "connect failed");
    }

    //收发缓冲区
    char send_buf[MAX_LINE];
    char receive_buf[MAX_LINE + 1];
    bzero(send_buf, MAX_LINE);
    bzero(receive_buf, MAX_LINE + 1);

    //接收发送
    yolanda_msgx("start to send and receive data.");
    while (fgets(send_buf, MAX_LINE, stdin) != NULL) {
        //准备数据
        size_t send_len = strlen(send_buf);
        if (send_buf[send_len - 1] == '\n') {
            //手动添加字符串结束符
            send_buf[send_len - 1] = '\0';
        }
        yolanda_msgx("now send: %s", send_buf);

        //发送
        int size_sent = send(socket_fd, send_buf, send_len, 0);
        if (size_sent < 0) {
            error(1, errno, "send failed");
        }
        yolanda_msgx("send %d bytes", size_sent);

        //接收
        //不要用 size_t 来接收，size_t 是 unsigned int 类型，而事实上，recvfrom 可能返回的是负数。
        int size_received = recv(
                socket_fd,
                receive_buf,
                MAX_LINE,
                0
        );
        if (size_received < 0) {
            error(1, errno, "recvfrom failed");
        }
        receive_buf[size_received] = '\0';
        yolanda_msgx("received size = %d, content =  %s", size_received, receive_buf);

        bzero(send_buf, MAX_LINE);
        bzero(receive_buf, MAX_LINE + 1);
    }

    exit(0);
}