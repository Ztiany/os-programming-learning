/*
 ============================================================================
 
 Author      : Ztiany
 Description : 演示向已经关闭的连接进行读操作【服务端】

 ============================================================================
 */
#include "stdio.h"
#include "stdbool.h"
#include <unistd.h>
#include <sys/socket.h>
#include "../lib/common.h"
#include "../lib/tcp_server.h"
#include "../lib/log.h"

int main(int argc, char **argv) {
    NO_BUFFER(stdin)
    NO_BUFFER(stdout)

    int connected_client_fd = tcp_server_accept_one(SERV_PORT);
    char buf[MAX_LINE];

    while (true) {
        bzero(buf, MAX_LINE);
        int read_size = read(connected_client_fd, buf, MAX_LINE);

        if (read_size < 0) {
            error(1, errno, "error read");
        } else if (read_size == 0) {
            error(1, 0, "client closed");
        }

        printf("read from client: %s\n", buf);

        sleep(10);

        int write_size = send(connected_client_fd, buf, read_size, 0);
        printf("send bytes: %d \n", write_size);
        if (write_size < 0) {
            error(1, errno, "error write");
        }
    }

}