/*
 ============================================================================

 Author      : Ztiany
 Description : 演示向已经关闭的连接进行读操作【客户端】

 ============================================================================
 */
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include "../lib/common.h"
#include "../lib/log.h"
#include "../lib/tcp_client.h"

int main(int argc, char **argv) {
    NO_BUFFER(stdin)
    NO_BUFFER(stdout)

    if (argc != 2) {
        error(1, 0, "usage: 17-write-closed-client  <IPaddress>");
    }

    int client_fd = tcp_client(argv[1], SERV_PORT);

    //注册 SIGPIPE 的信号处理程序，SIG_IGN 表示忽略该信号。
    signal(SIGPIPE, SIG_IGN);

    char *msg = "network programming";
    int n_written;
    int count = 10000000;

    while (count > 0) {
        n_written = send(client_fd, msg, strlen(msg), 0);
        printf("write %d bytes\n", n_written);

        if (n_written <= 0) {
            error(1, errno, "write error(%d)", errno);
            return EXIT_FAILURE;
        }

        count--;
    }

    return EXIT_SUCCESS;
}