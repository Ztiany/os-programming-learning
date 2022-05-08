#ifndef GEEKTIME_EVENT_LOOP_H
#define GEEKTIME_EVENT_LOOP_H

#include <pthread.h>
#include <stdbool.h>
#include "channel.h"
#include "channel_map.h"

#define CHANNEL_OP_ADD 1
#define CHANNEL_OP_DELETE 2
#define CHANNEL_OP_UPDATE 3

#define SOCKET_PAIR_WRITE 0
#define SOCKET_PAIR_READ 1

extern const struct event_dispatcher poll_event_dispatcher;

/** channel 节点，用以支持 event_loop 中采用链表存储 channel */
struct channel_node {
    /** CHANNEL_OP_ADD, CHANNEL_OP_DELETE or CHANNEL_OP_UPDATE*/
    int type;

    /** 当前节点的数据 */
    struct channel *channel;

    /** 下一个节点 */
    struct channel_node *next;
};

/** 事件循环器，一个 event_loop 必须绑定到一个线程，线程驱动 event_loop 不断地从 event_dispatcher 获取事件进行处理，周而复始 */
struct event_loop {
    /** 是否退出循环 */
    bool quit;

    /** 该循环器使用的 event_dispatcher */
    const struct event_dispatcher *dispatcher;

    /** 对应 dispatcher 需要的数据【由 dispatcher 提供，在需要的时候再交给 dispatcher】 */
    void *dispatcher_data;

    /** 在该循环器上注册的所有通道 */
    struct channel_map *channel_map;

    /* 队列的实现，为什么需要队列而不是直接将 channel 注册到 event_dispatcher 中呢？因为这涉及到两个操作：
     *
     *      1. 将新产生的 channel 与 event_loop 进行关联【即 channel 加入到 event_loop 的队列中】。
     *      2. 将新关联的  channel 注册到 event_loop 所绑定的 event_dispatcher 中。
     *
     * 步骤 1 可能运行在其他线程，而步骤 2 必须运行在 event_loop 所绑定的线程中。因此具体的做法是，当步骤 1 完成，判断当前运行的线程是否就是 event_loop 所绑定的线程，
     * 如果是则直接添加到 event_dispatcher 中，否则就唤醒  event_loop 所绑定的线程。而下面定义的 socket_pair 正是用于这个唤醒操作。
     *
     * 为什么步骤 2 必须运行在 event_loop 所绑定的线程中呢？我觉得有一下原因：
     *
     *      1. 让程序设计更合理，让 event_loop 自己处理自己的 channel。
     */
    /** 是否正在处理队列中的 channel【即向 event_dispatcher 注册 channel】*/
    bool is_handling_pending_channel;
    struct channel_node *pending_head;
    struct channel_node *pending_tail;

    /** event_loop 绑定的线程 */
    pthread_t owner_thread_id;
    /** event_loop 所绑定的线程的名称 */
    const char *thread_name;

    /* 线程安全相关*/
    pthread_mutex_t mutex;
    pthread_cond_t condition;

    /** socket_pair 用于该 event_loop 绑定的线程与其他线程进行通讯 */
    int socket_pair[2];
};

/** 初始化一个 event_loop，在哪个线程调用，event_loop 就绑定哪个线程 */
struct event_loop *event_loop_init();

/** 初始化一个 event_loop，并指定对应的线程名称 */
struct event_loop *event_loop_init_with_name(const char *thread_name);

/** 启动 event_loop，必须在 event_loop 所绑定的线程调用 */
int event_loop_run(struct event_loop *loop);

/** 唤醒参数指定的 event_loop 对应的线程 */
void event_loop_wakeup_thread(struct event_loop *loop);

/** 将需要添加到 event_dispatcher 的 channel 添加到 event_loop 的队列中 */
int event_loop_add_channel_event(struct event_loop *loop, int fd, struct channel *channel);

/** 将需要从 event_dispatcher 中删除的 channel 添加到 event_loop 的队列中 */
int event_loop_remove_channel_event(struct event_loop *loop, int fd, struct channel *channel);

/** 将需要更新的 channel 添加到 event_loop 的队列中 */
int event_loop_update_channel_event(struct event_loop *loop, int fd, struct channel *channel);

/** 处理队列中需要添加到 event_dispatcher 的 channel 【thread safe】*/
int event_loop_handle_pending_add(struct event_loop *loop, int fd, struct channel *channel);

/** 处理队列中需要从 event_dispatcher 删除的 channel 【thread safe】*/
int event_loop_handle_pending_remove(struct event_loop *loop, int fd, struct channel *channel);

/** 处理队列中需要从 event_dispatcher 更新的 channel 【thread safe】*/
int event_loop_handle_pending_update(struct event_loop *loop, int fd, struct channel *channel);

/** 当 event_dispatcher 收集完需要处理的事件后，通过该方法将其分发到对应的 event_loop 进行处理，events 为单个事件或多个事件的组合 */
int event_loop_active(struct event_loop *loop, int fd, int events);

#endif //GEEKTIME_EVENT_LOOP_H
