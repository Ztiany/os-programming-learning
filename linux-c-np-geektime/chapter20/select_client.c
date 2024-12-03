/*
 ============================================================================

 Author      : Ztiany
 Description : 演示 select 的使用

 ============================================================================
 */
#include <stdlib.h>
#include <stdbool.h>
#include "lib/read.h"
#include "../lib/common.h"
#include "../lib/tcp_client.h"

/* 请设置运行参数为 127.0.0.1 */
int main(int argc, char **argv) {
    NO_BUFFER(stdout)
    NO_BUFFER(stderr)

    if (argc != 2) {
        error(1, 0, "usage: 20-select-example <IPaddress>");
    }

    int client_fd = tcp_client(argv[1], SERV_PORT);

    char recv_buf[MAX_LINE + 1/*为了放置 '\0'*/];
    char send_buf[MAX_LINE];

    fd_set all_read;
    fd_set read_mask;
    FD_ZERO(&all_read);
    FD_SET(client_fd, &all_read);
    FD_SET(STDIN_FILENO, &all_read);

    while (true) {
        read_mask = all_read;
        if (select(client_fd + 1, &read_mask, NULL, NULL, NULL) < 0) {
            error(1, errno, "select error");
        }

        //读取服务端发来的数据
        if (FD_ISSET(client_fd, &read_mask)) {
            int length_read = read(client_fd, recv_buf, MAX_LINE);
            if (length_read < 0) {
                error(1, errno, "read error");
            } else if (length_read == 0) {
                error(1, 0, "server terminated \n");
            }

            recv_buf[length_read] = '\0';
            printf("received %s\n", recv_buf);
        }

        //读取标准输入
        if (FD_ISSET(STDIN_FILENO, &read_mask)) {
            //准备数据
            if (fgets(send_buf, MAX_LINE, stdin) == NULL) {
                continue;
            }

            //虽然 fgets 总是会添加一个 \0 放在最后，这里将 \n 替换，主要是为了打印信息易于阅读。
            int length_send = strlen(send_buf);
            if (send_buf[length_send - 1] == '\n') {
                send_buf[length_send - 1] = '\0';
            }
            printf("now sending: %s, and length = %d\n", send_buf, length_send);

            //写数据【这里是阻塞调用】
            int length_write = write(client_fd, send_buf, length_send);
            if (length_write < 0) {
                error(1, errno, "write error");
            }
            printf("has send %d bytes\n", length_write);
        }
    }

    exit(EXIT_SUCCESS);
}
