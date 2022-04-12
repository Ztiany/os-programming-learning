#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../lib/common.h"
#include "../lib/log.h"
#include "lib/read.h"

/* 请设置运行参数为 127.0.0.1 */
int main(int argc, char **argv) {
    if (argc != 2) {
        error(1, 0, "usage: 11-grace-client <IPaddress>");
    }

    // 创建客户端 Socket
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd < 0) {
        error(1, errno, "socket error");
    }

    // 创建服务器地址
    struct sockaddr_in server_addr;
    server_addr.sin_port = htons(SERV_PORT);
    server_addr.sin_family = AF_INET;
    //convert IPv4 and IPv6 addresses from text to binary form. returns 1 on success (network address was successfully converted).
    int pton_result = inet_pton(AF_INET, argv[1], &server_addr.sin_addr);
    if (pton_result != 1) {
        error(1, errno, "pton_result failed ");
    }

    // 连接服务器
    int connection_result = connect(client_fd, TO_SOCK_ADDR(server_addr), sizeof(server_addr));
    if (connection_result < 0) {
        error(1, errno, "connect error");
    }

    //收发缓冲区
    char send_line[MAX_LINE];
    char receive_line[MAX_LINE + 1];//加 1 位用于设置 '\0'
    size_t size_read;

    //select 描述符
    fd_set read_mask;
    fd_set all_reads; //读描述符集合
    FD_ZERO(&all_reads);// 初始化描述符集合
    //0 就是标准输入，这里表示对标准输入进行监听
    FD_SET(0, &all_reads);
    //这边表示对 client_fd 进行监听
    FD_SET(client_fd, &all_reads);

    //开始收发数据
    for (;;) {
        read_mask = all_reads;

        // 执行 select，只对读感兴趣
        int select_result = select(client_fd + 1, &read_mask, NULL, NULL, NULL);
        if (select_result < 0) {//如果有异常则报错退出
            error(1, errno, "select error");
        }

        // 当连接套接字上有数据可读，将数据读入到程序缓冲区中。
        if (FD_ISSET(client_fd, &read_mask)) {
            size_read = read(client_fd, receive_line, MAX_LINE);
            if (size_read < 0) {
                error(1, errno, "read error");
            }
            if (size_read == 0) {
                error(1, 0, "server terminated");
            }
            receive_line[size_read] = '\0';
            // 打印接收到的数据
            fputs(receive_line, stdout);
            fputs("\n", stdout);
        }

        //当标准输入上有数据可读，读入后进行判断。
        if (FD_ISSET(0, &read_mask)) {
            if (fgets(send_line, MAX_LINE, stdin) == NULL) {
                continue;
            }

            //如果输入的是“shutdown”，则关闭标准输入的 I/O 事件感知，并调用 shutdown 函数关闭写方向；
            if (strncmp(send_line, "shutdown", 8) == 0) {
                FD_CLR(0, &all_reads);
                if (shutdown(client_fd, SHUT_WR) < 0) {
                    error(1, errno, "shutdown error");
                }
            }
                //如果输入的是”close“，则调用 close 函数关闭连接；
            else if (strncmp(send_line, "close", 5) == 0) {
                FD_CLR(0, &all_reads);
                if (close(client_fd) < 0) {
                    error(1, errno, "close error");
                }
                //等待系统完成挥手
                sleep(6);
                exit(0);
            }
                //处理正常的输入，将回车符截掉，调用 write 函数，通过套接字将数据发送给服务器端。
            else {
                size_t size_to_write = strlen(send_line);
                if (send_line[size_to_write - 1] == '\n') {//将 fgets 可能读到的 \n 换成 字符串结束符【接收端可以安全打印】
                    send_line[size_to_write - 1] = 0;
                }
                printf("now sending %s\n", send_line);
                size_t size_written = write(client_fd, send_line, size_to_write);
                if (size_written < 0) {
                    error(1, errno, "write failed ");
                }
                printf("send bytes: %zu \n", size_written);
            }
        }
    }

}