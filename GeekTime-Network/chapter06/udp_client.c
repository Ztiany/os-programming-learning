#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../lib/log.h"
#include "../lib/common.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        error(1, 0, "usage: udp-client <IPaddress>");
    }

    //本地 socket
    int client_fd;
    client_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_fd < 0) {
        error(1, errno, "socket failed ");
    }

    //发送目标地址
    struct sockaddr_in remote_addr;
    bzero(&remote_addr, sizeof(remote_addr));
    remote_addr.sin_port = htons(SERV_PORT);
    remote_addr.sin_family = AF_INET;
    //convert IPv4 and IPv6 addresses from text to binary form. returns 1 on success (network address was successfully converted).
    int pton_result = inet_pton(AF_INET, argv[1], &remote_addr.sin_addr);
    if (pton_result != 1) {
        error(1, errno, "pton_result failed ");
    }

    //接受地址的来源地址
    struct sockaddr *replay_addr;
    replay_addr = malloc(sizeof(remote_addr));
    socklen_t replay_addr_len;
    char send_line[MAX_LINE], recv_line[MAX_LINE + 1];

    //  Upon successful completion, fgets() shall return s.
    while (fgets(send_line, MAX_LINE, stdin) != NULL) {
        //准备数据
        int real_len = strlen(send_line);
        if (send_line[real_len - 1] == '\n') {
            send_line[real_len - 1] = 0;
        }
        printf("now sending %s\n", send_line);

        //发送数据
        size_t rt = sendto(client_fd, send_line, strlen(send_line), 0, (struct sockaddr *) &remote_addr,
                           sizeof(remote_addr));
        if (rt < 0) {
            error(1, errno, "send failed ");
        }
        printf("send bytes: %zu \n", rt);

        replay_addr_len = 0;
        int received_len = recvfrom(client_fd, recv_line, MAX_LINE, 0, replay_addr, &replay_addr_len);
        if (received_len < 0) {
            error(1, errno, "recvfrom failed");
        }
        recv_line[received_len] = 0;
        fputs(recv_line, stdout);
        fputs("\n", stdout);
    }
}