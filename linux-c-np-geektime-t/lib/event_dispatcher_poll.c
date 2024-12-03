#include <sys/poll.h>
#include "event_dispatcher.h"
#include "log.h"

#define INIT_POLL_SIZE 1024

/* 没有用到*/
struct poll_idx {
    int idx_plus_one;
};

/** poll 需要的数据*/
struct poll_dispatcher_data {
    /** 没有用到*/
    int event_count;

    /** 没有用到*/
    int fds_number;

    struct pollfd *event_set;

    /** 没有用到*/
    int realloc_copy;

    /** 没有用到*/
    struct pollfd *event_set_copy;
};

/** 初始化 dispatcher */
static void *poll_init(struct event_loop *loop) {
    struct poll_dispatcher_data *data = malloc(sizeof(struct poll_dispatcher_data));

    //初始化 pollfd 数组，这个数组的第一个元素是 server_fd，其余的用来记录将要连接的 client_fd。
    data->event_set = malloc(sizeof(struct pollfd) * INIT_POLL_SIZE);

    for (int i = 0; i < INIT_POLL_SIZE; ++i) {
        data->event_set[i].fd = -1;//-1 表示没有被占用
    }

    //下面数据其实都没有被用到
    data->event_count = 0;
    data->fds_number = 0;
    data->realloc_copy = 0;
    data->event_set_copy = NULL;

    return data;
}

/** 将一个 channel 注册到该 event_dispatcher 上 */
static int poll_add(struct event_loop *loop, struct channel *channel) {
    //获取事件集
    struct poll_dispatcher_data *data = (struct poll_dispatcher_data *) loop->dispatcher_data;

    //设置感兴趣的事件
    short events = 0;
    if (channel_event_is_readable(channel)) {
        events |= POLLRDNORM;
    }
    if (channel_event_is_writeable(channel)) {
        events |= POLLWRNORM;
    }

    //找到一个可用的位置并设置进去
    int fd = channel->fd;
    for (int i = 0; i < INIT_POLL_SIZE; ++i) {
        if (data->event_set[i].fd < 0) {
            data->event_set[i].fd = fd;
            data->event_set[i].events = events;
            break;
        }
    }

    yolanda_msgx("poll-dispatcher added channel(fd=%d, events=%d) in thread(%s)", fd, events, loop->thread_name);

    return 1;
}

/** 将一个 channel 从 event_dispatcher 上删除 */
static int poll_delete(struct event_loop *loop, struct channel *channel) {
    struct poll_dispatcher_data *data = (struct poll_dispatcher_data *) loop->dispatcher_data;
    int fd = channel->fd;

    //找到对应的记录
    bool deleted = false;
    for (int i = 0; i < INIT_POLL_SIZE; ++i) {
        if (data->event_set[i].fd == fd) {
            data->event_set[i].fd = -1;
            deleted = true;
            break;
        }
    }

    if (deleted) {
        yolanda_msgx("poll-dispatcher deleted channel(fd=%d) in thread(%s)", fd, loop->thread_name);
    } else {
        yolanda_errorx("poll-dispatcher tried to delete channel(fd=%d) in thread(%s), but not found.", fd,
                       loop->thread_name);
    }

    return 1;
}

/** 更新已经在 event_dispatcher 上注册的 channel */
static int poll_update(struct event_loop *loop, struct channel *channel) {
    //获取事件集
    struct poll_dispatcher_data *data = (struct poll_dispatcher_data *) loop->dispatcher_data;

    //设置感兴趣的事件
    short events = 0;
    if (channel_event_is_readable(channel)) {
        events |= POLLRDNORM;
    }
    if (channel_event_is_writeable(channel)) {
        events |= POLLWRNORM;
    }

    //找到对应的记录
    bool updated = false;
    int fd = channel->fd;
    for (int i = 0; i < INIT_POLL_SIZE; ++i) {
        if (data->event_set[i].fd == fd) {
            data->event_set[i].events = events;
            updated = true;
            break;
        }
    }

    if (updated) {
        yolanda_msgx("poll-dispatcher updated channel(fd=%d) in thread(%s)", fd, loop->thread_name);
    } else {
        yolanda_errorx("poll-dispatcher tried to update channel(fd=%d) in thread(%s), but not found.", fd,
                       loop->thread_name);
    }

    return 1;
}

/** 在该 event_dispatcher 上等待事件，并进行分发【比如调用 select 方法】*/
static int poll_dispatch(struct event_loop *loop, struct timeval *timeout) {
    //获取事件集
    struct poll_dispatcher_data *data = (struct poll_dispatcher_data *) loop->dispatcher_data;

    //指定 poll
    int ready_number;
    int time_wait = timeout->tv_sec * 1000;
    if ((ready_number = poll(data->event_set, INIT_POLL_SIZE, time_wait)) < 0) {
        error(1, errno, "thread(%s) do dispatch at poll-dispatcher error.", loop->thread_name);
    }

    if (ready_number == 0) {
        return 0;
    }

    //分发事件
    int socket_fd;
    for (int i = 0; i < INIT_POLL_SIZE; ++i) {
        //TODO：优化避免栈空间分配
        struct pollfd poll_fd = data->event_set[i];
        socket_fd = poll_fd.fd;

        //过滤掉空的 fd
        if (socket_fd < 0) {
            continue;
        }

        //根据 revents 分发事件
        if (poll_fd.revents > 0) {
            //TODO：处理 POLLERR 事件
            //分发可读事件
            if (poll_fd.revents & POLLRDNORM) {
                yolanda_msgx(
                        "poll-dispatcher dispatch READ EVENT to channel(fd=%d) in thread(%s)",
                        socket_fd,
                        loop->thread_name
                );
                event_loop_active(loop, socket_fd, EVENT_READ);
            }
            //分发可写事件
            if (poll_fd.revents & POLLWRNORM) {
                yolanda_msgx(
                        "poll-dispatcher dispatch WRITE EVENT to channel(fd=%d) in thread(%s)",
                        socket_fd,
                        loop->thread_name
                );
                event_loop_active(loop, socket_fd, EVENT_WRITE);
            }

            //处理完了就直接结束，不需要遍历整个集合。
            if (--ready_number <= 0) {
                break;
            }
        }

    }

    return 1;
}

/** 清空 event_loop 在该 event_dispatcher 上注册的所有事件 */
static void poll_clear(struct event_loop *loop) {
    yolanda_msgx("poll-dispatcher free data in thread(%s)", loop->thread_name);
    struct poll_dispatcher_data *data = (struct poll_dispatcher_data *) loop->dispatcher_data;
    free(data->event_set);
    data->event_set = NULL;
    free(data);
    loop->dispatcher_data = NULL;
}

const struct event_dispatcher poll_event_dispatcher = {
        "poll",
        poll_init,
        poll_add,
        poll_delete,
        poll_update,
        poll_dispatch,
        poll_clear
};