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

#endif //YOLANDA_COMMON_H
