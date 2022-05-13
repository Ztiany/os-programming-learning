#include <sys/epoll.h>
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

static int epoll_delete(struct event_loop *loop, struct channel *channel) {}

static int epoll_update(struct event_loop *loop, struct channel *channel) {}

static int epoll_dispatch(struct event_loop *loop, struct timeval *timeout) {}

static void epoll_clear(struct event_loop *loop) {}

const struct event_dispatcher poll_event_dispatcher = {
        "epoll",
        epoll_init,
        epoll_add,
        epoll_delete,
        epoll_update,
        epoll_dispatch,
        epoll_clear
};