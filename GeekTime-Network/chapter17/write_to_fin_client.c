/*
 ============================================================================
 
 Author      : Ztiany
 Description : 演示向已经关闭的连接进行读写操作【客户端】

 ============================================================================
 */
#include <sys/socket.h>
#include <unistd.h>
#include "../lib/common.h"
#include "../lib/log.h"
#include "../lib/tcp_client.h"

int main(int argc, char **argv) {
    NO_BUFFER(stdin)
    NO_BUFFER(stdout)

    if (argc != 2) {
        error(1, 0, "usage: 17-write-to-fin-client  <IPaddress>");
    }

    int sock = tcp_client(argv[1], SERV_PORT);
    char buf[MAX_LINE];
    bzero(buf, MAX_LINE);
    int send_length;
    int read_length;

    while (fgets(buf, MAX_LINE, stdin) != NULL) {
        send_length = strlen(buf);
        if (send_length == 0) {
            continue;
        }
        if (buf[send_length - 1] == '\n') {
            buf[send_length - 1] = '\0';
        }

        send_length = send(sock, buf, send_length, 0);
        if (send_length < 0) {
            error(1, errno, "send error");
        }

        sleep(3);

        bzero(buf, MAX_LINE);
        read_length = read(sock, buf, MAX_LINE);
        if (read_length < 0) {
            error(1, errno, "read error");
        } else if (read_length == 0) {
            error(1, 0, "peer connection closed");
        } else {
            printf("%s\n", buf);
        }

        bzero(buf, MAX_LINE);
    }

}