#include "read.h"

size_t readn(int fd, void *buffer, size_t size) {
    char *buffer_pointer = buffer;
    int length = size;

    while (length > 0) {
        int result = read(fd, buffer_pointer, length);

        if (result < 0) {
            if (errno == EINTR) {
                // 1. 遇到 EINTR 时，实际上并没有发生任何错误；
                // 2. 考虑非阻塞的情况，这里需要再次调用 read。
                continue;
            } else {
                return (-1);
            }
        } else if (result == 0) {
            /* EOF(End of File) 表示套接字关闭 */
            break;
        }

        length -= result;
        buffer_pointer += result;
    }

    /* 返回的是实际读取的字节数*/
    return (size - length);
}

size_t read_message(int fd, char *buffer, size_t length) {
    u_int32_t message_length;
    u_int32_t message_type;

    //first: read message length
    int read_count = readn(fd, &message_length, sizeof(u_int32_t));
    if (read_count != sizeof(u_int32_t)) {
        return read_count < 0 ? -1 : 0;
    }
    message_length = ntohl(message_length);//网络字节序转换为本地字节序
    yolanda_msgx("message length: %d", message_length);

    //second: read message type
    read_count = readn(fd, &message_type, sizeof(u_int32_t));
    if (read_count != sizeof(u_int32_t)) {
        return read_count < 0 ? -1 : 0;
    }
    message_type = ntohl(message_type);
    yolanda_msgx("message type: %d", message_type);

    //third: check if buffer is enough
    if (length < message_length) {
        return -1;
    }

    //fourth: read message body
    read_count = readn(fd, buffer, message_length);
    if (read_count != message_length) {
        return read_count < 0 ? -1 : 0;
    }

    return read_count;
}

size_t read_line1(int fd, char *buffer, size_t length) {
    int count_up = 0;
    char c = '\0';
    int read_result = 0;

    while ((count_up < length - 1/*为 \0 留一个位置*/) && (c != '\n')) {
        read_result = recv(fd, &c, 1, 0);

        if (read_result > 0) {//读到了数据

            if (c == '\r') {//如果是回车符，则看下个字符是不是换行符
                read_result = recv(fd, &c, 1, MSG_PEEK);//MSG_PEEK 表示读取，但是不移动读取指针。
                if (read_result > 0 && c == '\n') {//读到了 \r\n
                    recv(fd, &c, 1, 0);//这回是真正的去读 \n，则最后 \r\n 中的 \r 被消除，留下了 \n。
                } else {//只读到 \r，而不是 \r\n，则把 \r 改为 \h，然后结束。
                    c = '\n';
                }
            }

            buffer[count_up] = c;
            count_up++;

        } else {//读取错误
            c = '\n';
        }
    }

    //最后不管是读到了换行符，还是读取了足够的长度，都在末尾加上一个结束符。
    buffer[count_up] = '\0';
    return count_up;
}