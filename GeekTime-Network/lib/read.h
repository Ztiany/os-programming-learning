#ifndef GEEKTIME_READ_H
#define GEEKTIME_READ_H

#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include "log.h"

/**尝试从 fd 读取指定 size 的数据*/
size_t readn(int fd, void *vptr, size_t n);

/**
 *  按照下面的定义读取数据：\n\n
 *
 *   struct { \n
 *   &nbsp&nbsp    u_int32_t length; \n
 *   &nbsp&nbsp    u_int32_t type; \n
 *   &nbsp&nbsp    char data[BUFFER_SIZE]; \n
 *   } message;
 *
 */
size_t read_message(int fd, char *buffer, size_t length);

/** 读取一行的数据【也就是读到 \\r, \\n 或者 \\r\\n 为止】，buffer 始终以 \0 结束，注意：该版本是逐个字节读取，效率较低。*/
size_t read_line(int fd, char *buffer, size_t length);

/** 读取一行的数据【也就是读到 \\r, \\n 或者 \\r\\n 为止】，buffer 始终以 \0 结束。*/
size_t read_line_buffered(int fd, char *buffer, size_t length);

#endif //GEEKTIME_READ_H
