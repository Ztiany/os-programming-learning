/*
 ============================================================================

 Author      : Ztiany
 Description : 演示 poll 的使用

 ============================================================================
 */
#include <stdlib.h>
#include <stdbool.h>
#include <poll.h>
#include "lib/read.h"
#include "../lib/common.h"
#include "../lib/tcp_server.h"

#define INIT_SIZE 128

int main(int argc, char *argv[]) {
    NO_BUFFER(stdout)
    NO_BUFFER(stderr)

    int server_fd = tcp_server_listen(SERV_PORT);

    //准备必要的数据
    int ready_fd_count;
    char recv_buf[MAX_LINE];
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = 0;

    //初始化 poll 的数据集
    struct pollfd event_set[INIT_SIZE];
    event_set[0].fd = server_fd;
    event_set[0].events = POLLRDNORM;//POLLRDNORM ，表示我们期望系统内核检测监听套接字上的连接建立完成事件。
    for (int i = 1; i < INIT_SIZE; ++i) {
        // 用 -1 表示这个数组位置还没有被占用
        event_set[i].fd = -1;
    }

    while (true) {
        //执行 poll【timeout 设置为 -1，表示在 I/O 事件发生之前 poll 调用一直阻塞】
        if ((ready_fd_count = poll(event_set, INIT_SIZE, -1)) < 0) {
            error(1, errno, "poll error");
        }

        //处理服务端的 fd
        if ((event_set[0].revents & POLLRDNORM) != 0) {
            bzero(&client_addr, sizeof(client_addr));
            client_addr_len = sizeof(client_addr);
            int connected_fd = accept(server_fd, TO_SOCK_ADDR(client_addr), &client_addr_len);
            yolanda_msgx("a new client(%d) has been accepted", connected_fd);
            //找到一个可以记录该连接套接字的位置
            int index;
            for (index = 1; index < INIT_SIZE; ++index) {
                if (event_set[index].fd < 0) {
                    event_set[index].fd = connected_fd;
                    event_set[index].events = POLLRDNORM;
                    break;
                }
            }

            //如果全部数据集都用完了，就报错。【这里数组的大小固定为 INIT_SIZE，这在实际的生产环境肯定是需要改进的】
            if (index == INIT_SIZE) {
                error(1, 0, "can not hold so many clients");
            }

            //此次 poll 的可操作 fd 都全部处理完了，【这是一个加速优化能力，因为 poll 返回的一个整数，说明了这次 I/O 事件描述符的个数，
            // 如果处理完监听套接字之后，就已经完成了这次 I/O 复用所要处理的事情，那么我们就可以跳过后面的处理，再次进入 poll 调用。】
            if (--ready_fd_count <= 0) {
                continue;
            }
        }

        //处理其他数据集
        for (int i = 1; i < INIT_SIZE; ++i) {
            //过滤掉不可操作的 fd
            int client_fd = event_set[i].fd;
            if (client_fd < 0) {
                continue;
            }

            //处理可读或者出错的 fd
            if ((event_set[i].revents & (POLLRDNORM | POLLERR))) {
                int read_count = read(client_fd, recv_buf, sizeof(recv_buf));
                if (read_count > 0) {
                    if (write(client_fd, recv_buf, read_count) < 0) {
                        error(1, errno, "write to fd(%d) error", client_fd);
                    }
                } else if (read_count == 0 || errno == ECONNRESET/*表示连接已经关闭*/) {
                    close(client_fd);
                    event_set[i].fd = -1;
                    yolanda_msgx("a client(%d) has exited", client_fd);
                } else {
                    error(1, errno, "read from fd(%d) error", client_fd);
                }
            }

            //此次 poll 的可操作 fd 都全部处理完了
            if (--ready_fd_count <= 0) {
                continue;
            }
        }

    }
}