#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/uio.h>
#include "butter.h"

const char *CRLF = "\r\n";

struct buffer *buffer_new() {
    struct buffer *buffer = malloc(sizeof(struct buffer));
    if (buffer == NULL) {
        return NULL;
    }

    buffer->data = malloc(INIT_BUFFER_SIZE);
    buffer->write_index = buffer->read_index = 0;
    buffer->total_size = INIT_BUFFER_SIZE;

    return buffer;
}

void buffer_free(struct buffer *buffer) {
    if (buffer == NULL) {
        return;
    }
    free(buffer->data);
    free(buffer);
}

int buffer_writeable_size(struct buffer *buffer) {
    return buffer->total_size - buffer->write_index;
}

int buffer_readable_size(struct buffer *buffer) {
    return buffer->write_index - buffer->read_index;
}

int buffer_front_spare_size(struct buffer *buffer) {
    return buffer->read_index;
}

void make_room(struct buffer *buffer, int size) {
    if (buffer_writeable_size(buffer) >= size) {
        return;
    }

    //如果 front_spare 和 writeable 的大小加起来可以容纳数据，则把可读数据往前面拷贝。
    if (buffer_front_spare_size(buffer) + buffer_writeable_size(buffer) >= size) {
        int readable_size = buffer_readable_size(buffer);
        for (int i = 0; i < readable_size; i++) {
            memcpy(buffer->data + i, buffer->data + buffer->read_index + i, 1);
        }

        buffer->read_index = 0;
        buffer->write_index = readable_size;
        return;
    }

    //对缓冲区进行扩容
    void *data_temp = realloc(buffer->data, buffer->total_size + size);
    if (data_temp == NULL) {
        return;
    }
    buffer->data = data_temp;
    buffer->total_size = buffer->total_size + size;
}

int buffer_append(struct buffer *buffer, const void *data, int size) {
    if (data == NULL) {
        return false;
    }
    if (size <= 0) {
        return false;
    }

    make_room(buffer, size);
    memcpy(buffer->data + buffer->write_index, data, size);
    buffer->write_index += size;
    return true;
}

int buffer_append_char(struct buffer *buffer, char data) {
    make_room(buffer, sizeof(char));
    buffer->data[buffer->write_index++] = data;
    return true;
}

int buffer_append_string(struct buffer *buffer, const char *str) {
    if (str == NULL) {
        return false;
    }
    buffer_append(buffer, str, strlen(str));
    return true;
}

int buffer_socket_read(struct buffer *buffer, int socket_fd) {
    /* extra buffer for preventing the original buffer from being not enough. */
    char additional_buffer[INIT_BUFFER_SIZE];
    struct iovec vec[2];

    int maximum_writeable = buffer_writeable_size(buffer);
    vec[0].iov_base = buffer->data + buffer->write_index;
    vec[0].iov_len = maximum_writeable;

    vec[1].iov_base = additional_buffer;
    vec[1].iov_len = INIT_BUFFER_SIZE;

    //从 socket 读取数据。
    int read_size = readv(socket_fd, vec, 2/*length of iovec array.*/);
    if (read_size < 0) {//出错就返回 -1
        return -1;
    }

    //读取数据时，只用到了 buffer。
    if (read_size <= maximum_writeable) {
        buffer->write_index += read_size;
    } else {//读取数据时，buffer 被独满，且还用用到了额外的空间。
        buffer->write_index = buffer->total_size;
        buffer_append(buffer, additional_buffer, read_size - maximum_writeable);
    }
    return read_size;
}

char buffer_read_char(struct buffer *buffer) {
    char result = buffer->data[buffer->read_index];
    buffer->read_index++;
    return result;
}

char *buffer_find_CRLF(struct buffer *buffer) {
    /* The memmem() function returns a pointer to the beginning of the substring, or NULL if the substring is not found. */
    /* reference: https://man7.org/linux/man-pages/man3/memmem.3.html */
    //return memmem((void*)buffer->data + buffer->read_index, buffer_readable_size(buffer),(void*) CRLF, 2);

    //looks like memmem is not working, so use strstr instead.
    return strstr(buffer->data + buffer->read_index, CRLF);
}