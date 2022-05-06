/*
 ============================================================================

 Author      : Ztiany
 Description : NIO + 多路复用服务器：缓冲区。

 ============================================================================
 */

#ifndef GEEKTIME_BUTTER_H
#define GEEKTIME_BUTTER_H

// 64k
#define INIT_BUFFER_SIZE 65536

/** 数据缓冲区 */
struct buffer {
    /** 指向一块内存（实际的缓冲区） */
    char *data;
    /** 当前已经读到的位置 */
    int read_index;
    /** 当前已经写到的位置 */
    int write_index;
    /** 缓冲区大小 */
    int total_size;
};

/** 创建一个缓冲区 */
struct buffer *buffer_new();

/** 释放缓冲区*/
void buffer_free(struct buffer *buffer);

/** 获取缓冲区还可以写入数据的大小 */
int buffer_writeable_size(struct buffer *buffer);

/** 获取缓冲区中有多少读取数据可以读取 */
int buffer_readable_size(struct buffer *buffer);

/** 获取缓冲区中闲置的空间大小【闲置的空间：一块空间先被写入，然后读取，那么这块空间在被重置之前就是处于闲置状态】 */
int buffer_front_spare_size(struct buffer *buffer);

/** 对缓冲区进行扩容，使得其能够读取大小为 size 的数据【如果缓冲区本身的空间足够，则不会扩容】 */
void make_room(struct buffer *buffer, int size);

/** 向缓冲区中写入指定大小的数据 */
int buffer_append(struct buffer *buffer, const void *data, int size);

/** 向缓冲区中写入一个字符*/
int buffer_append_char(struct buffer *buffer, char data);

/** 向缓冲区中写入一个字符串【字符串必须是 \\0 结尾】*/
int buffer_append_string(struct buffer *buffer, const char *str);

/** 从 socket 中读取数据到缓冲区中 */
int buffer_socket_read(struct buffer *buffer, int socket_fd);

/** 从缓冲区中读取一个字符 */
char buffer_read_char(struct buffer *buffer);

/** 返回一个指针，该指针指向缓冲区中第一个 \\r\\n 所在位置，找不到则返回 NULL */
char *buffer_find_CRLF(struct buffer *buffer);

#endif //GEEKTIME_BUTTER_H
