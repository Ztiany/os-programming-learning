#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "tcp_client.h"
#include "common.h"
#include "log.h"

int tcp_client(const char *address, int port) {

    // 创建客户端 Socket
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd < 0) {
        error(1, errno, "tcp_client socket() failed");
    }

    // 创建服务器地址
    struct sockaddr_in server_addr;
    server_addr.sin_port = htons(port);
    server_addr.sin_family = AF_INET;
    //convert IPv4 and IPv6 addresses from text to binary form. returns 1 on success (network address was successfully converted).
    int pton_result = inet_pton(AF_INET, address, &server_addr.sin_addr);
    if (pton_result != 1) {
        error(1, errno, "tcp_client inet_pton() failed ");
    }

    // 连接服务器
    int connection_result = connect(client_fd, TO_SOCK_ADDR(server_addr), sizeof(server_addr));
    if (connection_result < 0) {
        error(1, errno, "tcp_client connect() failed");
    }

    return client_fd;
}