#ifndef GEEKTIME_WRITE_H
#define GEEKTIME_WRITE_H

#include <stdlib.h>

/** 向文件描述符 fd 写入 n 字节数 */
ssize_t writen(int fd, const void *data, size_t n);

#endif //GEEKTIME_WRITE_H
