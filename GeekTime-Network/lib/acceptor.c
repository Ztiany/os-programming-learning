#include "acceptor.h"
#include "sys/socket.h"
#include <netinet/in.h>
#include <obstack.h>
#include "log.h"

struct acceptor *acceptor_init(int port) {
    // 分配一个 acceptor
    struct acceptor *acceptor = malloc(sizeof(struct acceptor));
    if (acceptor == NULL) {
        error(1, errno, "acceptor_init malloc(acceptor) failed");
    }
    acceptor->server_port = port;

    // 创建套接字
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        error(1, errno, "acceptor_init socket() failed");
    }
    make_nonblocking(server_fd);//设置为非阻塞
    acceptor->server_fd = server_fd;

    // 设置地址可重用
    int on = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    // 创建套接字地址
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // 绑定到本地
    if (bind(server_fd, TO_SOCK_ADDR(server_addr), sizeof(server_addr)) < 0) {
        error(1, errno, "acceptor_init bind() failed");
    }

    // 开始监听
    if (listen(server_fd, LISTEN_Q) < 0) {
        error(1, errno, "acceptor_init listen() failed");
    }

    return acceptor;
}

void acceptor_free(struct acceptor *acceptor) {
    free(acceptor);
}