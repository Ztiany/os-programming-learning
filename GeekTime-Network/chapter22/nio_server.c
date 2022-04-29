/*
 ============================================================================

 Author      : Ztiany
 Description : 演示非阻塞 I/O + select 多路复用。

 ============================================================================
 */
#include <stdlib.h>
#include <stdbool.h>
#include "lib/read.h"
#include "../lib/common.h"
#include "../lib/tcp_server.h"

#define FD_INIT_SIZE 128

/**
 * ROT13（回转 13 位，rotateby13places，有时中间加了个减号称作ROT-13）是一种简易的置换暗码。
 * ROT-13 编码是一种每一个字母被另一个字母代替的方法。这个代替字母是由原来的字母向前移动 13 个字母而得到的。数字和非字母字符保持不变。
 * 它是一种在网路论坛用作隐藏八卦、妙句、谜题解答以及某些脏话的工具，目的是逃过版主或管理员的匆匆一瞥。
 * ROT13 激励了广泛的线上书信撰写与字母游戏，且它常于新闻群组对话中被提及。
 */
char rot13_char(char c) {
    if ((c >= 'a' && c <= 'm') || (c >= 'A' && c <= 'M'))
        return c + 13;
    else if ((c >= 'n' && c <= 'z') || (c >= 'N' && c <= 'Z'))
        return c - 13;
    else
        return c;
}

/** 数据缓冲区 */
struct Buffer {
    /**socket fd*/
    int connect_fd;
    /**缓冲区*/
    char buffer[MAX_LINE];
    /**缓冲区读取位置*/
    size_t write_index;
    /**缓冲区写入位置*/
    size_t read_index;
    /**缓冲区是否可读【即缓冲区是否已经读入了数据】*/
    bool readable;
};

struct Buffer *new_buffer() {
    struct Buffer *buffer = malloc(sizeof(struct Buffer));
    if (buffer == NULL) {
        return NULL;
    }
    buffer->connect_fd = -1;
    buffer->read_index = buffer->write_index = 0;
    buffer->readable = false;
    return buffer;
}

void delete_buffer(struct Buffer *buffer) {
    free(buffer);
}

int on_socket_readable(int index, struct Buffer *buffer);

int on_socket_writeable(int index, struct Buffer *buffer);

int main(int argc, char **argv) {
    //创建初始化服务器【非阻塞模式】
    int server_fd = tcp_server_listen_non_blocking(SERV_PORT);

    //为客户端初始化缓冲区
    struct Buffer *buffers[FD_INIT_SIZE];
    for (int i = 0; i < FD_INIT_SIZE; ++i) {
        buffers[i] = new_buffer();
    }

    //select 必要的数据
    int max_fd;
    fd_set read_set, write_set, exception_set/*暂时没用处理 exception。*/;

    //开心循环 select
    while (true) {
        //重置数据集
        max_fd = server_fd;
        FD_ZERO(&read_set);
        FD_ZERO(&write_set);
        FD_ZERO(&exception_set);

        //设置 server_fd 的数据集
        FD_SET(server_fd, &read_set);
        //设置已经接收的客户端的数据集
        for (int i = 0; i < FD_INIT_SIZE; ++i) {
            if (buffers[i]->connect_fd < 0) {
                continue;
            }
            if (buffers[i]->connect_fd > max_fd) {//找到最大的那个 fd
                max_fd = buffers[i]->connect_fd;
            }
            FD_SET(buffers[i]->connect_fd, &read_set);//设置对读感兴趣
            if (buffers[i]->readable) {//如果已经读入了数据，则需要回写到客户端，因此设置对写感兴趣
                FD_SET(buffers[i]->connect_fd, &write_set);
            }
        }

        //执行 select
        if (select(max_fd + 1, &read_set, &write_set, &exception_set, NULL) < 0) {
            error(1, errno, "select error");
        }

        //===================================
        // 处理服务端情况
        //===================================
        if (FD_ISSET(server_fd, &read_set)) {
            printf("listening socket readable\n");
            sleep(5);//模拟服务器大量并发

            //准备必要数据
            struct sockaddr_in client_addr;
            bzero(&client_addr, sizeof(client_addr));
            socklen_t client_addr_len = sizeof(client_addr);
            int connected_fd = accept(server_fd, TO_SOCK_ADDR(client_addr), &client_addr_len);

            //对连接的客户端进行处理
            if (connected_fd < 0) {
                error(1, errno, "accept error");
            } else if (connected_fd > FD_INIT_SIZE) {/*超出了可处理能力【这里待优化】*/
                close(connected_fd);
                error(1, 0, "too many connections");
            } else {
                make_nonblocking(connected_fd);//设置非阻塞 I/O
                if (buffers[connected_fd]->connect_fd < 0) {
                    buffers[connected_fd]->connect_fd = connected_fd;
                } else {
                    error(1, 0, "duplicated fd appears");
                }
            }
        }

        //===================================
        // 处理客户端情况
        //===================================
        for (int i = 0; i < max_fd; ++i) {
            //过滤掉无效连接
            int fd = buffers[i]->connect_fd;
            if (fd < 0) {
                continue;
            }
            int result;
            if (FD_ISSET(fd, &read_set)) {
                result = on_socket_readable(i, buffers[i]);
            }
            if (FD_ISSET(fd, &write_set)) {
                result = on_socket_writeable(i, buffers[i]);
            }
            if (result != 0) {
                buffers[i]->connect_fd = -1;
                close(fd);
            }
        }
    }//end

}

int on_socket_readable(int index, struct Buffer *buffer) {

    return 1;
}

int on_socket_writeable(int index, struct Buffer *buffer) {
    return 1;
}