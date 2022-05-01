/*
 ============================================================================
 
 Author      : Ztiany
 Description : 基于多进程的 tcp server 实现

 ============================================================================
 */
#include <stdlib.h>
#include <stdbool.h>
#include <sys/wait.h>
#include "lib/read.h"
#include "../lib/common.h"
#include "../lib/chars.h"
#include "../lib/tcp_server.h"

void child_run(int fd);

/**
 * 注册了一个信号处理函数，用来回收子进程资源。函数 sigchld_handler，在一个循环体内调用了 waitpid 函数，以便回收所有已终止的子进程。这里选项 WNOHANG 用来告诉内核，
 * 即使还有未终止的子进程也不要阻塞在 waitpid 上。注意这里不可以使用 wait，因为 wait 函数在有未终止子进程的情况下，没有办法不阻塞。
 */
void child_exit_signal_handler() {
    yolanda_msgx("child_exit_signal_handler() started");
    while (waitpid(-1, 0, WNOHANG) > 0);
    yolanda_msgx("child_exit_signal_handler() ended");
}

/*客户端使用 telnet 即可。*/
int main(int argc, char **argv) {
    //创建一个 tcp server
    int server_fd = tcp_server_listen(SERV_PORT);

    //处理子进程退出
    signal(SIGCHLD, child_exit_signal_handler);

    while (true) {
        struct sockaddr_in child_addr;
        bzero(&child_addr, sizeof(child_addr));
        socklen_t child_addr_len = sizeof(child_addr);
        int client_fd = accept(server_fd, TO_SOCK_ADDR(child_addr), &child_addr_len);
        if (client_fd < 0) {
            error(1, errno, "accept error");
        }

        /*
         * 从父进程派生出的子进程，同时也会复制一份描述字，也就是说，连接套接字和监听套接字的引用计数都会被加 1，
         * 而调用 close 函数则会对引用计数进行减 1 操作，这样在套接字引用计数到 0 时，才可以将套接字资源回收。
         * 所以，这里的 close 函数非常重要，缺少了它们，就会引起服务器端资源的泄露。
         */
        if (fork() == 0) {//child process
            // 子进程不需要关心监听套接字，故而在这里关闭掉监听套接字 listen_fd
            close(server_fd);
            child_run(client_fd);
            exit(EXIT_SUCCESS);
        } else {
            // 进入的是父进程处理逻辑，父进程不需要关心连接套接字，所以在这里关闭连接套接字。
            close(client_fd);
        }
    }

    return EXIT_SUCCESS;
}

void child_run(int fd) {
    char receive_buf[MAX_LINE];
    char receive;
    int read_count;
    size_t out_buf_used = 0;

    while (true) {
        read_count = recv(fd, &receive, 1, 0);
        if (read_count == 0) {
            break;
        }
        if (read_count == -1) {
            perror("child read error.");
            break;
        }

        /* We do this test to keep the user from overflowing the buffer. */
        if (out_buf_used < sizeof(receive_buf)) {
            receive_buf[out_buf_used++] = rot13_char(receive);
        }

        if (receive == '\n') {
            send(fd, receive_buf, out_buf_used, 0);
            out_buf_used = 0;
            continue;
        }
    }
}
