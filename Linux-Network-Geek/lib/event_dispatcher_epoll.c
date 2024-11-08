#include <sys/epoll.h>
#include <unistd.h>
#include "event_dispatcher.h"
#include "log.h"

#define MAX_EVENTS 128

typedef struct {
    int event_count;
    int fd_number;
    int realloc_copy;
    int epoll_fd;
    struct epoll_event *epoll_events;
} epoll_dispatcher_data;

static void *epoll_init(struct event_loop *loop) {
    epoll_dispatcher_data *data = malloc(sizeof(epoll_dispatcher_data));

    data->event_count = 0;
    data->epoll_events = NULL;
    data->fd_number = 0;

    //准备 epoll 的数据
    data->epoll_fd = epoll_create1(0);
    if (data->epoll_fd < 0) {
        error(1, errno, "epoll_create1 failed");
    }
    data->epoll_events = calloc(MAX_EVENTS, sizeof(struct epoll_event));

    return data;
}

static int epoll_add(struct event_loop *loop, struct channel *channel) {
    epoll_dispatcher_data *data = (epoll_dispatcher_data *) loop->dispatcher_data;
    int fd = channel->fd;

    int events = 0;
    if (channel_event_is_readable(channel)) {
        events = events | EPOLLIN;
    }
    if (channel_event_is_writeable(channel)) {
        events = events | EPOLLOUT;
    }

    struct epoll_event event;
    event.data.fd = fd;
    event.events = events;

    if (epoll_ctl(data->epoll_fd, EPOLL_CTL_ADD, fd, &event) == -1) {
        error(1, errno, "epoll_ctl add  fd failed");
    }

    return 1;
}

static int epoll_delete(struct event_loop *loop, struct channel *channel) {
    epoll_dispatcher_data *data = (epoll_dispatcher_data *) loop->dispatcher_data;
    int fd = channel->fd;

    int events = 0;
    if (channel_event_is_readable(channel)) {
        events = events | EPOLLIN;
    }
    if (channel_event_is_writeable(channel)) {
        events = events | EPOLLOUT;
    }

    struct epoll_event event;
    event.data.fd = fd;
    event.events = events;

    if (epoll_ctl(data->epoll_fd, EPOLL_CTL_DEL, fd, &event) == -1) {
        error(1, errno, "epoll_ctl delete  fd failed");
    }

    return 1;
}

static int epoll_update(struct event_loop *loop, struct channel *channel) {
    epoll_dispatcher_data *data = (epoll_dispatcher_data *) loop->dispatcher_data;
    int fd = channel->fd;

    int events = 0;
    if (channel_event_is_readable(channel)) {
        events = events | EPOLLIN;
    }
    if (channel_event_is_writeable(channel)) {
        events = events | EPOLLOUT;
    }

    struct epoll_event event;
    event.data.fd = fd;
    event.events = events;

    if (epoll_ctl(data->epoll_fd, EPOLL_CTL_MOD, fd, &event) == -1) {
        error(1, errno, "epoll_ctl update  fd failed");
    }

    return 1;
}

static int epoll_dispatch(struct event_loop *loop, struct timeval *timeout) {
    epoll_dispatcher_data *data = (epoll_dispatcher_data *) loop->dispatcher_data;
    int epoll_result = epoll_wait(data->epoll_fd, data->epoll_events, MAX_EVENTS, -1);
    yolanda_msgx("thread(%s) do dispatch at epoll-dispatcher return %d.", loop->thread_name, epoll_result);

    for (int i = 0; i < epoll_result; ++i) {
        // error
        // 其实这里不会被调用，因为 EPOLLIN, EPOLLOUT, EPOLLHUP, EPOLLRDHUP 都需要主动在 epoll_ctal 时加入到 events 中，而上面并没有注册。
        // 如果不处理 EPOLLRDHUP 的话，也可以处理 EPOLLIN 事件，此时 read 返回 0, 同样表明对端已经关闭。
        if ((data->epoll_events[i].events & EPOLLERR) || (data->epoll_events[i].events & EPOLLHUP)) {
            yolanda_msgx(
                    "epoll-dispatcher dispatch ERROR to channel(fd=%d) in thread(%s)",
                    data->epoll_events[i].data.fd,
                    loop->thread_name
            );
            close(data->epoll_events[i].data.fd);
            continue;
        }

        // read
        if (data->epoll_events[i].events & EPOLLIN) {
            yolanda_msgx(
                    "epoll-dispatcher dispatch READ EVENT to channel(fd=%d) in thread(%s)",
                    data->epoll_events[i].data.fd,
                    loop->thread_name
            );
            event_loop_active(loop, data->epoll_events[i].data.fd, EVENT_READ);
        }

        // write
        if (data->epoll_events[i].events & EPOLLOUT) {
            yolanda_msgx(
                    "epoll-dispatcher dispatch WRITE EVENT  to channel(fd=%d) in thread(%s)",
                    data->epoll_events[i].data.fd,
                    loop->thread_name
            );
            event_loop_active(loop, data->epoll_events[i].data.fd, EVENT_WRITE);
        }
    }

    return 1;
}

static void epoll_clear(struct event_loop *loop) {
    epoll_dispatcher_data *data = (epoll_dispatcher_data *) loop->dispatcher_data;
    free(data->epoll_events);
    close(data->epoll_fd);
    free(data);
    loop->dispatcher_data = NULL;
}

const struct event_dispatcher epoll_event_dispatcher = {
        "epoll",
        epoll_init,
        epoll_add,
        epoll_delete,
        epoll_update,
        epoll_dispatch,
        epoll_clear
};