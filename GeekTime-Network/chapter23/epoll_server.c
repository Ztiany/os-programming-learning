/*
 ============================================================================
 
 Author      : Ztiany
 Description : 使用 epoll 实现一个服务器

 ============================================================================
 */
#include <stdlib.h>
#include <stdbool.h>
#include <sys/epoll.h>
#include "lib/read.h"
#include "../lib/common.h"
#include "../lib/tcp_server.h"

#define MAX_EVENTS 128

int main(int argc, char **argv) {
    NO_BUFFER(stdout)
    NO_BUFFER(stderr)

    //创建非阻塞服务器
    int server_fd = tcp_server_listen_non_blocking(SERV_PORT);

    //创建 epoll 需要的数据
    struct epoll_event event;
    struct epoll_event *events;
    int epoll_return;
    int epoll_fd = epoll_create1(0);
    if (epoll_fd < 0) {
        error(1, errno, "epoll_create1 failed");
    }

    //设置 epoll 事件
    event.data.fd = server_fd;
    event.events = EPOLLIN | EPOLLET;//edge-triggered
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event) == 1) {
        error(1, errno, "epoll_ctl failed");
    }

    /* 用于接收 epoll 返回的事件 */
    events = calloc(MAX_EVENTS, sizeof(event));

    //用于接收数据
    char receive_buf[512];

    //开始循环 epoll
    while (true) {
        epoll_return = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        yolanda_msgx("epoll wake up with %d", epoll_return);

        //循环处理所有返回的事件
        for (int i = 0; i < epoll_return; ++i) {

            //情况 1：对应的 fd 出错
            if ((events[i].events & EPOLLERR) //错误
                || (events[i].events & EPOLLHUP) //被挂起
                || (!(events[i].events & EPOLLIN))/*不可读*/) {
                yolanda_msgx("epoll error");
                close(events[i].data.fd);
                continue;
            }

            //情况 2：服务器事件
            if (server_fd == events[i].data.fd) {
                struct sockaddr_in client_addr;
                socklen_t client_addr_len = sizeof(client_addr);
                int client_fd = accept(server_fd, TO_SOCK_ADDR(client_addr), &client_addr_len);
                if (client_fd < 0) {
                    error(1, errno, "accept error");
                } else {
                    make_nonblocking(client_fd);
                    event.data.fd = client_fd;
                    event.events = EPOLLIN | EPOLLET; //edge-triggered
                    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event) == 1) {
                        error(1, errno, "epoll_ctl for client(%d) failed", client_fd);
                    }
                    yolanda_msgx("a new client(%d) has been accepted", client_fd);
                }
                continue;
            }

            //情况 3：处理客户端事件
            int processing_fd = events[i].data.fd;
            yolanda_msgx("now, process client(%d)'s event", processing_fd);
            while (true) {
                bzero(receive_buf, sizeof(receive_buf));
                int read_size = read(processing_fd, receive_buf, sizeof(receive_buf));
                yolanda_msgx("received %d bytes from client(%d)", read_size, processing_fd);

                //读取发送错误
                if (read_size < 0) {
                    /* If errno == EAGAIN, that means we have read all data.*/
                    if (errno != EAGAIN) {
                        close(processing_fd);
                        error(1, errno, "read for client(%d) error", processing_fd);
                    }
                    break;
                }

                //对端已经关闭
                if (read_size == 0) {
                    close(processing_fd);
                    break;
                }

                //把读取到的数据全部写回去
                if (write(processing_fd, receive_buf, read_size) < 0) {
                    error(1, errno, "write to client(%d) error", processing_fd);
                }
            }//while end

        }
    }

    //释放资源
    free(events);
    close(epoll_fd);
}