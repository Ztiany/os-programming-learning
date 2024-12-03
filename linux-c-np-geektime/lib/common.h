#ifndef YOLANDA_COMMON_H
#define YOLANDA_COMMON_H

#include "config.h"

#ifdef EPOLL_ENABLE

#include    <sys/epoll.h>

#endif

#define    SERV_PORT      43211
#define    MAX_LINE        4096
#define    UNIX_STR_PATH   "/var/lib/unixstream.sock"
#define    LISTEN_Q        1024
#define    BUFFER_SIZE    4096

#define    KEEP_ALIVE_TIME  10
#define    KEEP_ALIVE_INTERVAL  3
#define    KEEP_ALIVE_PROBE_TIMES  3

#define TO_SOCK_ADDR(addr) (struct sockaddr *)(&addr)
#define P_TO_SOCK_ADDR(addr_p) (struct sockaddr *)(addr_p)

/** make the fd nonblocking.*/
void make_nonblocking(int fd);

// 远程调试时，写入到 stdout 和 stderr 中的数据可能会保存在缓冲区中，导致客户端没有输出，通过下面的宏定义，可以让缓冲区不工作。
#define NO_BUFFER(out) setbuf(out, NULL);

#endif //YOLANDA_COMMON_H
