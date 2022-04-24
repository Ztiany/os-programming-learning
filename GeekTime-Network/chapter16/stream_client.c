#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../lib/common.h"
#include "../lib/log.h"

/* 请设置运行参数为 127.0.0.1 */
int main(int argc, char **argv) {
    NO_BUFFER(stdout)
    NO_BUFFER(stderr)

    if (argc != 2) {
        error(1, 0, "usage: 16-stream-client <IPaddress>");
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
    //convert IPv4 and IPv6 addresses from text to binary form. returns 1 on success (network address was successfully converted).
    int pton_result = inet_pton(AF_INET, argv[1], &server_addr.sin_addr);
    if (pton_result != 1) {
        error(1, errno, "pton_result failed ");
    }

    // 连接服务器
    int connection_result = connect(client_fd, TO_SOCK_ADDR(server_addr), sizeof(server_addr));
    if (connection_result < 0) {
        error(1, errno, "connect error");
    }

    // 定义数据流格式
    struct {
        u_int32_t length;
        u_int32_t type;
        char data[BUFFER_SIZE];
    } message;

    bzero(&message, sizeof(message));

    // 开始发送数据
    while (fgets(message.data, sizeof(message.data), stdin) != NULL) {
        // 准备数据
        size_t message_length = strlen(message.data);
        message.length = htonl(message_length);
        message.type = htonl(1);
        // 发送数据
        size_t send_length = sizeof(message.length) + sizeof(message.type) + message_length;
        int send_result = send(client_fd, (char *) &message, send_length, 0);
        if (send_result < 0) {
            error(1, errno, "send error");
        }

        bzero(&message, sizeof(message));
    }

    exit(0);
}