#include "event_dispatcher.h"
#include <sys/poll.h>

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
static void poll_init(struct event_loop *event_loop) {
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
}

/** 将一个 channel 注册到该 event_dispatcher 上 */
static void poll_add(struct event_loop *event_loop, struct channel *channel);

/** 将一个 channel 从 event_dispatcher 上删除 */
static void poll_delete(struct event_loop *event_loop, struct channel *channel);

/** 更新已经在 event_dispatcher 上注册的 channel */
static void poll_update(struct event_loop *event_loop, struct channel *channel);

/** 在该 event_dispatcher 上等待事件，并进行分发【比如调用 select 方法】*/
static void poll_dispatch(struct event_loop *event_loop, struct timeval *timeout);

/** 清空 event_loop 在该 event_dispatcher 上注册的所有事件 */
static void poll_clear(struct event_loop *event_loop);

const struct event_dispatcher poll_event_dispatcher = {
        "poll",
        poll_init,
        poll_add,
        poll_delete,
        poll_update,
        poll_dispatch,
        poll_clear
};