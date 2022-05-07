#ifndef GEEKTIME_CHANNEL_H
#define GEEKTIME_CHANNEL_H

#include <stdbool.h>

/* 下面定义的数字支持位操作 */
/** when time is out. */
#define EVENT_TIMEOUT 0x01
/** wait for a fd to be writeable. */
#define EVENT_READ 0x02
/** wait for a fd to be readable. */
#define EVENT_WRITE 0x04
/** wait for a POSIX signal to be raised. */
#define EVENT_SIGNAL    0x08

/** 通道可读时的回调。 */
typedef int(*event_read_callback)(void *data);

/** 通道可写时的回调。 */
typedef int(*event_write_callback)(void *data);

struct channel {

    /** a file descriptor related to this channel. */
    int fd;

    /** EVENT_TIMEOUT, EVENT_READ, EVENT_WRITE,EVENT_SIGNAL 中的任意组合*/
    int events;

    /* callback */
    event_read_callback on_channel_readable;
    event_write_callback on_channel_writeable;

    /** callback data, 可能是 event_loop, tcp_server 或者 tcp_connection. */
    void *data;
};

/** 创建一个 channel */
struct channel *channel_new(
        int fd,
        int events,
        event_read_callback read_callback,
        event_write_callback write_callback,
        void *data
);

/** 释放 channel */
void channel_free(struct channel *channel);

/** 该 channel 是否可写*/
bool channel_event_is_writeable(struct channel *channel);

/** 将该 channel 设置为可写*/
void channel_event_enable_write(struct channel *channel);

/** 将该 channel 设置为不可写*/
void channel_event_disable_write(struct channel *channel);

#endif //GEEKTIME_CHANNEL_H
