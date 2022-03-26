#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../lib/log.h"

void send_data(int);

//1000M
# define MESSAGE_SIZE 10240000

int main(int argc, char **argv) {
    int client_fd;
    struct sockaddr_in remote_addr;
    if (argc != 2) {
        error(1, 0, "usage: tcp-client <IPaddress>");
    }

    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd < 0) {
        error(1, errno, "socket failed ");
    }

    bzero(&remote_addr, sizeof(remote_addr));
    remote_addr.sin_port = htons(12345);
    remote_addr.sin_family = AF_INET;
    //convert IPv4 and IPv6 addresses from text to binary form. returns 1 on success (network address was successfully converted).
    int pton_result = inet_pton(AF_INET, argv[1], &remote_addr.sin_addr);
    if (pton_result != 1) {
        error(1, errno, "pton_result failed ");
    }

    int connect_result = connect(client_fd, (struct sockaddr *) &remote_addr, sizeof(remote_addr));
    if (connect_result < 0) {
        error(1, errno, "connect failed ");
    }

    send_data(client_fd);

    return EXIT_SUCCESS;
}

void send_data(int fd) {
    //准备数据
    char *query;
    query = malloc(MESSAGE_SIZE + 1);
    for (int i = 0; i < MESSAGE_SIZE; ++i) {
        query[i] = 'a';
    }
    query[MESSAGE_SIZE] = '\0';

    const char *cp;
    cp = query;
    size_t remaining = strlen(query);

    //发送数据
    while (remaining) {
        int size_written = send(fd, cp, remaining, 0);
        fprintf(stdout, "send into buffer %d \n", size_written);
        if (size_written <= 0) {
            error(1, errno, "send failed\n");
            return;
        }
        remaining -= size_written;
        cp += size_written;
    }

    yolanda_debugx("send finished");
}