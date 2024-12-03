#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/uio.h>
#include "../lib/common.h"
#include "../lib/log.h"

/*
 * 1. 请设置运行参数为 127.0.0.1
 * 2. 服务器可以直接使用 chapter11 中的 server 程序。
*/
int main(int argc, char **argv) {
    if (argc != 2) {
        error(1, 0, "usage: 12-ping-client <IPaddress>");
    }

    // 创建客户端 Socket
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd < 0) {
        error(1, errno, "socket error");
    }

    // 创建服务器地址
    struct sockaddr_in server_addr;
    server_addr.sin_port = htons(SERV_PORT);
    server_addr.sin_family = AF_INET;
    // convert IPv4 and IPv6 addresses from text to binary form. returns 1 on success (network address was successfully converted).
    int pton_result = inet_pton(AF_INET, argv[1], &server_addr.sin_addr);
    if (pton_result != 1) {
        error(1, errno, "pton_result failed ");
    }

    // 连接服务器
    int connection_result = connect(client_fd, TO_SOCK_ADDR(server_addr), sizeof(server_addr));
    if (connection_result < 0) {
        error(1, errno, "connect error");
    }

    //发送数据的缓冲区，使用 iovec 数组，分别写入了两个不同的字符串，一个是“hello,”，另一个通过标准输入读入。
    struct iovec iov[2];
    char *send_buf_one = "hello, ";
    char send_buf_tow[128];
    bzero(send_buf_tow, sizeof(send_buf_tow));

    iov[0].iov_base = send_buf_one;
    iov[0].iov_len = strlen(send_buf_one);//这里不要使用 htonl。

    while (fgets(send_buf_tow, sizeof(send_buf_tow), stdin) != NULL) {
        iov[1].iov_base = send_buf_tow;
        iov[1].iov_len =strlen(send_buf_tow);

        yolanda_msgx("read stdin: %s", send_buf_tow);

        // 发送数据
        size_t send_result = writev(client_fd, iov, 2);
        if (send_result < 0) {
            error(1, errno, "writev error");
        }

        bzero(send_buf_tow, sizeof(send_buf_tow));
    }

    return EXIT_SUCCESS;
}