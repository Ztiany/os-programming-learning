#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include "write.h"

ssize_t writen(int fd, const void *data, size_t n) {
    size_t left_count;
    ssize_t size_written;
    const char *ptr;

    ptr = data;
    left_count = n;

    // 如果还有数据没被拷贝完成，就一直循环。
    while (left_count > 0) {
        if ((size_written = write(fd, ptr, left_count)) <= 0) {
            /* 这里 EINTR 是非阻塞 non-blocking 情况下，通知我们再次调用 write() */
            if (size_written < 0 && errno == EINTR)
                size_written = 0;
            else
                /* 出错退出 */
                return -1;
        }

        /* 指针增大，剩下字节数变小 */
        left_count -= size_written;
        ptr += size_written;
    }

    return n;
}