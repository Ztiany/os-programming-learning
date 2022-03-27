#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../lib/log.h"
#include "../lib/common.h"

/*请设置运行参数为 127.0.0.1*/
int main(int argc, char **argv) {
    NO_BUFFER(stdout)
    NO_BUFFER(stderr)

    if (argc != 2) {
        error(1, 0, "usage: udp-client <IPaddress>");
    }

    //本地 socket
    int client_fd;
    client_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_fd < 0) {
        error(1, errno, "socket failed ");
    }

    //服务器目标地址，SERV_PORT 表示发送到所有网卡
    struct sockaddr_in remote_addr;
    bzero(&remote_addr, sizeof(remote_addr));
    remote_addr.sin_port = htons(SERV_PORT);
    remote_addr.sin_family = AF_INET;
    //convert IPv4 and IPv6 addresses from text to binary form. returns 1 on success (network address was successfully converted).
    int pton_result = inet_pton(AF_INET, argv[1], &remote_addr.sin_addr);
    if (pton_result != 1) {
        error(1, errno, "pton_result failed ");
    }

    //响应方地址
    struct sockaddr *replay_addr;
    replay_addr = malloc(sizeof(remote_addr));
    socklen_t replay_addr_len;
    char send_line[MAX_LINE], recv_line[MAX_LINE + 1];

    //  Upon successful completion, fgets() shall return s. 如果 fgets() 会将读取到的 \n 放到 s 中。
    while (fgets(send_line, MAX_LINE, stdin) != NULL) {
        //准备数据【从控制台输入】
        size_t real_len = strlen(send_line);
        if (send_line[real_len - 1] == '\n') {
            send_line[real_len - 1] = 0;
        }
        printf("now sending %s\n", send_line);

        //发送数据
        size_t size_sent = sendto(client_fd, send_line, strlen(send_line), 0, TO_SOCK_ADDR(remote_addr),
                                  sizeof(remote_addr));
        if (size_sent < 0) {
            error(1, errno, "send failed ");
        }
        printf("send bytes: %zu \n", size_sent);

        //接收数据
        replay_addr_len = 0;
        size_t received_len = recvfrom(client_fd, recv_line, MAX_LINE, 0, replay_addr, &replay_addr_len);
        if (received_len < 0) {
            error(1, errno, "recvfrom failed");
        }
        recv_line[received_len] = 0;

        //输出接收到的数据
        fputs(recv_line, stdout);
        fputs("\n", stdout);
    }
}