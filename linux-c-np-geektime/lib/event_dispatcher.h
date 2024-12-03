#ifndef GEEKTIME_EVENT_DISPATCHER_H
#define GEEKTIME_EVENT_DISPATCHER_H

#include <sys/socket.h>
#include "channel.h"
#include "event_loop.h"

/** 抽象的 I/O 事件分发器，其实现可以为 select, poll 或者 epoll */
struct event_dispatcher {
    /**  select, poll 或者 epoll */
    const char *name;

    /** 初始化 dispatcher */
    void* (*init)(struct event_loop *event_loop);

    /** 将一个 channel 注册到该 event_dispatcher 上 */
    int (*add)(struct event_loop *event_loop, struct channel *channel);

    /** 将一个 channel 从 event_dispatcher 上删除 */
    int (*delete)(struct event_loop *event_loop, struct channel *channel);

    /** 更新已经在 event_dispatcher 上注册的 channel */
    int (*update)(struct event_loop *event_loop, struct channel *channel);

    /** 在该 event_dispatcher 上等待事件，并进行分发【比如调用 select 方法】*/
    int (*dispatch)(struct event_loop *event_loop, struct timeval *timeout);

    /** 清空 event_loop 在该 event_dispatcher 上注册的所有事件 */
    void (*clear)(struct event_loop *event_loop);
};

#endif //GEEKTIME_EVENT_DISPATCHER_H
