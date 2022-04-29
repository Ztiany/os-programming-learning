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
    /**指向缓冲区中已经被使用的区域的末尾。*/
    size_t write_index;
    /**指向缓冲区中已经被消耗的区域的末尾【read_index <= write_index-1】*/
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

int on_socket_readable(int fd, struct Buffer *buffer);

int on_socket_writeable(int fd, struct Buffer *buffer);

int main(int argc, char **argv) {
    NO_BUFFER(stdout)
    NO_BUFFER(stderr)

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
            yolanda_msgx("set read_set for  client(%d)", buffers[i]->connect_fd);
            FD_SET(buffers[i]->connect_fd, &read_set);//设置对读感兴趣
            if (buffers[i]->readable) {//如果已经读入了数据，则需要回写到客户端，因此设置对写感兴趣
                yolanda_msgx("set write_set for  client(%d)", buffers[i]->connect_fd);
                FD_SET(buffers[i]->connect_fd, &write_set);
            }
        }

        yolanda_msgx("max_fd = %d", max_fd);

        //执行 select
        int selected = select(max_fd + 1, &read_set, &write_set, &exception_set, NULL);
        if (selected < 0) {
            error(1, errno, "select error");
        }
        yolanda_msgx("selected for %d", server_fd);

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
                yolanda_msgx("A new client(%d) has been accepted.", connected_fd);
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
        for (int i = 0; i < max_fd + 1; ++i) {
            //过滤掉服务端本身和无效连接
            int fd = buffers[i]->connect_fd;
            if (fd < 0 || fd == server_fd) {
                continue;
            }

            int result;
            if (FD_ISSET(fd, &read_set)) {
                yolanda_msgx("invoke read for client(%d)", fd);
                result = on_socket_readable(fd, buffers[i]);
            }
            if (FD_ISSET(fd, &write_set)) {
                yolanda_msgx("invoke write for client(%d)", fd);
                result = on_socket_writeable(fd, buffers[i]);
            }
            if (result != 0) {
                yolanda_msgx("A client(%d) has exited.", fd);
                buffers[i]->connect_fd = -1;
                close(fd);
            }
        }
    }//end

}


int on_socket_readable(int fd, struct Buffer *buffer) {
    char read_buffer[1024];
    int read_count;

    while (true) {
        read_count = read(fd, read_buffer, sizeof(read_buffer));
        yolanda_msgx("client(%d) read for %d.", fd, read_count);

        if (read_count <= 0) {
            break;
        }

        //逐个字符地把读到的数据放到缓冲区中
        for (int i = 0; i < read_count; ++i) {
            if (buffer->write_index < sizeof(buffer->buffer)) {//判断容量
                buffer->buffer[buffer->write_index] = rot13_char(read_buffer[i]);
                buffer->write_index++;
            } else{
                //这里其实有问题【TODO：会导致很多数据丢失，待后续优化】
                yolanda_msgx("abandoned 1");
            }
            //如果读取了回车符，则认为 client 端发送结束，此时可以把编码后的数据回送给客户端。
            if (read_buffer[i] == '\n') {
                buffer->readable = true;
            }
        }
    }

    if (read_count == 0) {//对端已经关闭
        return 1;//返回 1 表示退出该客户端
    } else if (read_count < 0) {//出错
        if (errno == EAGAIN) {//表示当前已经没有数据，等待下一次通知。
            return 0;//返回 0 表示下次继续。
        }
        return -1;//返回 -1 表示退出该客户端。
    }

    return 0;//返回 0 表示下次继续。
}

int on_socket_writeable(int fd, struct Buffer *buffer) {
    int write_size;
    while (buffer->read_index < buffer->write_index) {
        write_size = send(fd, buffer->buffer + buffer->read_index, buffer->write_index - buffer->read_index, 0);
        yolanda_msgx("client(%d) write for %d bytes.", fd, write_size);

        if (write_size < 0) {
            if (errno == EAGAIN) {
                return 0;//返回 0 表示下次再来。
            }
            return -1;
        }
        buffer->read_index += write_size;
    }

    //缓冲区中读取的所有数据都已经回写完到客户端了
    if (buffer->read_index == buffer->write_index) {
        yolanda_msgx("client(%d) write all it buffer for %d bytes.", fd, write_size);
        buffer->write_index = buffer->read_index = 0;
        buffer->readable = false;
    }

    return 0;
}