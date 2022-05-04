/*
 ============================================================================
 
 Author      : Ztiany
 Description : 基于线程模型的 server

 ============================================================================
 */
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include "lib/read.h"
#include "../lib/common.h"
#include "../lib/tcp_server.h"

extern void loop_echo(int);

void *handle_client(void *args) {
    //分离线程
    pthread_detach(pthread_self());
    int client_fd = *((int *) args);

    printf("start a thread(%lu) to handle client(%d).\n", pthread_self(), client_fd);

    loop_echo(client_fd);
    return NULL;
}

/*客户端使用 telnet 即可。*/
int main(int argc, char **argv) {
    NO_BUFFER(stdout)
    NO_BUFFER(stderr)

    //创建一个 tcp server
    int server_fd = tcp_server_listen(SERV_PORT);

    while (true) {
        struct sockaddr_in child_addr;
        bzero(&child_addr, sizeof(child_addr));
        socklen_t child_addr_len = sizeof(child_addr);
        int client_fd = accept(server_fd, TO_SOCK_ADDR(child_addr), &child_addr_len);
        if (client_fd < 0) {
            error(1, errno, "accept error");
        }

        yolanda_msgx("a new client(%d) has been accepted.", client_fd);

        //启动子线程去处理该客户端
        pthread_t tid;
        pthread_create(&tid, NULL, handle_client, (void *) (&client_fd));
    }

    return EXIT_SUCCESS;
}
