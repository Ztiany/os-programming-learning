#ifndef GEEKTIME_READ_H
#define GEEKTIME_READ_H

#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

/**尝试从 fd 读取指定 size 的数据*/
size_t readn(int fd, void *vptr, size_t n);

#endif //GEEKTIME_READ_H
