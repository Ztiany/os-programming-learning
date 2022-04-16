#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../lib/common.h"
#include "../lib/log.h"
#include "lib/read.h"
#include "message_protocol.h"

/* 请设置运行参数为 127.0.0.1 */
int main(int argc, char **argv) {
    if (argc != 2) {
        error(1, 0, "usage: 12-ping-client <IPaddress>");
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
    // convert IPv4 and IPv6 addresses from text to binary form. returns 1 on success (network address was successfully converted).
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
    char receive_line[MAX_LINE + 1];//加 1 位用于设置 '\0'
    size_t size_read;

    // 心跳包设置
    struct timeval timeout;//心跳包时间
    int heartbeat_count = 0;//心跳次数
    timeout.tv_sec = KEEP_ALIVE_TIME;
    timeout.tv_usec = 0;

    //select 描述符
    fd_set read_mask;
    fd_set all_reads; //读描述符集合
    FD_ZERO(&all_reads);// 初始化描述符集合
    //0 就是标准输入，这里表示对标准输入进行监听
    FD_SET(0, &all_reads);
    //这边表示对 client_fd 进行监听
    FD_SET(client_fd, &all_reads);

    //消息
    messageObject messageObject;

    //开始收发数据
    for (;;) {
        read_mask = all_reads;

        // 执行 select，只对读感兴趣【这里设置了超时时间为 KEEP_ALIVE_TIME，这相当于保活时间】
        int select_result = select(client_fd + 1, &read_mask, NULL, NULL, &timeout/*超时时间*/);
        if (select_result < 0) {//如果有异常则报错退出
            error(1, errno, "select error");
        }

        // 当 select 超时，还没有任何可读的数据，则发送心跳包。
        if (select_result == 0) {
            if (++heartbeat_count > KEEP_ALIVE_PROBE_TIMES) {//心跳包连续没有得到恢复，则认为断连了。
                error(1, 0, "server is dead");
            }
            yolanda_msgx("send heartbeat, current count: %d", heartbeat_count);
            messageObject.type = htonl(MSG_PING);
            size_t size_sent = send(client_fd, (char *) &messageObject, sizeof(messageObject), 0);
            if (size_sent < 0) {
                error(1, errno, "send error");
            }
            timeout.tv_sec = KEEP_ALIVE_INTERVAL;//一旦开始心跳，就变为 3 秒发一次
            continue;
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

            // 客户端在接收到服务器端程序之后的处理。为了简单，这里就没有再进行报文格式的转换和分析。在实际的工作中，这里其实是需要对报文进行解析后处理的，
            // 只有是 PONG 类型的回应，我们才认为是 PING 探活的结果。这里认为既然收到服务器端的报文，那么连接就是正常的，所以会对探活计数器和探活时间都置零，
            // 等待下一次探活时间的来临。

            // 打印接收到的数据
            yolanda_msgx("a message has arrived. reset the heartbeat count to 0.");
            // 重置心跳次数
            timeout.tv_sec = KEEP_ALIVE_TIME;
            heartbeat_count = 0;
        }
    }

}