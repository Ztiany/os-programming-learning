#include <unistd.h>
#include <assert.h>
#include "event_loop.h"
#include "event_dispatcher.h"
#include "log.h"
#include "utils.h"

//=============================================
// 线程间通讯
//=============================================

void event_loop_wakeup_thread(struct event_loop *loop) {
    char write_one = 'a';
    size_t wrote_size = write(loop->socket_pair[SOCKET_PAIR_WRITE], &write_one, sizeof(write_one));
    if (wrote_size != sizeof(write_one)) {
        yolanda_errorx("notify_loop_thread: send signal to thread(%s)  failed", loop->thread_name);
    } else {
        yolanda_msgx("notify_loop_thread: send signal to thread(%s)  successfully", loop->thread_name);
    }
}

int handle_loop_thread_wakeup(void *data) {
    struct event_loop *loop = (struct event_loop *) data;
    //这里读取的数据与上面 event_loop_wakeup_thread 方法写入的数据对应，重点不在于写入什么数据，而是要达到唤醒的目的。
    char read_one;
    size_t read_size = read(loop->socket_pair[SOCKET_PAIR_READ], &read_one, sizeof(read_one));

    if (read_size != sizeof(read_one)) {
        yolanda_errorx("handle_loop_thread_wakeup: thread(%s)  wakeup failed", loop->thread_name);
    } else {
        yolanda_msgx("handle_loop_thread_wakeup: thread(%s) wakeup  successfully", loop->thread_name);
    }

    return true;
}


//=============================================
// 处理队列中的通道
//=============================================

void event_loop_handle_pending_channel(struct event_loop *loop) {
    //因为操作共享数据 event_loop 的队列，所以这里加锁
    pthread_mutex_lock(&loop->mutex);
    loop->is_handling_pending_channel = true;

    //处理队列中所有的 channel
    struct channel_node *node = loop->pending_head;
    while (node != NULL) {

        struct channel *pending_channel = node->channel;
        int channel_fd = pending_channel->fd;

        if (node->type == CHANNEL_OP_ADD) {
            event_loop_handle_pending_add(loop, channel_fd, pending_channel);
        } else if (node->type == CHANNEL_OP_DELETE) {
            event_loop_handle_pending_remove(loop, channel_fd, pending_channel);
        } else if (node->type == CHANNEL_OP_UPDATE) {
            event_loop_handle_pending_update(loop, channel_fd, pending_channel);
        }
        node = node->next;
    }

    //处理完了，清空队列。
    loop->pending_head = loop->pending_tail = NULL;
    loop->is_handling_pending_channel = false;

    //释放锁
    pthread_mutex_unlock(&loop->mutex);
}

int event_loop_handle_pending_add(struct event_loop *loop, int fd, struct channel *channel) {
    yolanda_msgx("thread(%s): register channel(fd = %d) to dispatcher", loop->thread_name, fd);
    if (fd < 0) {
        return 0;
    }

    //------------------------------------------------------------
    // 完成 channel 与 fd 的映射关系
    //------------------------------------------------------------
    //step1：如果需要，扩容映射空间
    struct channel_map *map = loop->channel_map;
    if (fd >= map->length) {
        if (!channel_map_make_space(map, fd, sizeof(struct channel *)/*这里是 channel 指针，而不是 channel 本身*/)) {
            return -1;
        }
    }
    //step1：如果没有添加过，就添加到映射中，并注册 channel 的事件。
    if (map->entities[fd] == NULL) {
        map->entities[fd] = channel;
        //向 dispatcher 注册 channel 感兴趣的事件
        loop->dispatcher->add(loop, channel);
    }

    return 1;
}

int event_loop_handle_pending_remove(struct event_loop *loop, int fd, struct channel *channel) {
    yolanda_msgx("thread(%s): remove channel(fd = %d) from loop and dispatcher", loop->thread_name, fd);

    if (fd < 0) {
        return 0;
    }

    struct channel_map *map = loop->channel_map;
    //这个应该不会发生，检测是为了防止程序崩溃
    if (fd > map->length) {
        return -1;
    }

    //找到之前的注册的 channel
    struct channel *added_channel = map->entities[fd];
    if (added_channel == NULL) {
        yolanda_errorx("event_loop_handle_pending_remove: fd(%d) has no related channel", fd);
        return -1;
    }

    //从 dispatcher 移除该 channel
    const struct event_dispatcher *dispatcher = loop->dispatcher;
    int result = dispatcher->delete(loop, added_channel);

    //清空映射关系
    map->entities[fd] = NULL;

    //返回结果
    return result;
}

int event_loop_handle_pending_update(struct event_loop *loop, int fd, struct channel *channel) {
    yolanda_msgx("thread(%s): update channel(fd = %d) at event_loop", loop->thread_name, fd);

    if (fd < 0) {
        return 0;
    }

    struct channel_map *map = loop->channel_map;

    //这个应该不会发生，检测是为了防止程序崩溃
    if (fd > map->length) {
        return -1;
    }

    //检测之前注册的 channel
    if (map->entities[fd] == NULL) {
        yolanda_errorx("event_loop_handle_pending_update: fd(%d) has no related channel", fd);
        return -1;
    }

    //执行更新
    const struct event_dispatcher *dispatcher = loop->dispatcher;
    return dispatcher->update(loop, channel);
}


//=============================================
// 添加、移除或更新新的通道
//=============================================

int event_loop_pending_channel_enqueue_no_lock(struct event_loop *loop, int fd, struct channel *channel, int op_type) {
    struct channel_node *node = malloc(sizeof(struct channel_node));
    if (node == NULL) {
        return -1;
    }
    node->type = op_type;
    node->channel = channel;
    node->next = NULL;

    if (loop->pending_head == NULL) {//队列为 null
        loop->pending_head = loop->pending_tail = node;
    } else {//队列不为 null
        loop->pending_tail->next = node;//将新的 node 添加到队尾
        loop->pending_tail = node;//修改队尾为最新的 node
    }

    return 1;
}

int event_loop_do_channel_event(struct event_loop *loop, int fd, struct channel *channel, int op_type) {
    //因为操作共享数据 event_loop 的队列，所以这里加锁
    pthread_mutex_lock(&loop->mutex);
    //线程校验，此时不应该在处理队列中的 channel
    assert(!loop->is_handling_pending_channel);
    //pending channel 入队
    int result = event_loop_pending_channel_enqueue_no_lock(loop, fd, channel, op_type);
    //释放锁
    pthread_mutex_unlock(&loop->mutex);

    //本身就在这个线程，则直接处理。
    if (is_in_same_thread(loop)) {
        event_loop_handle_pending_channel(loop);
    } else {//否则通知 loop 所对应的线程。
        event_loop_wakeup_thread(loop);
    }

    return result;
}

int event_loop_add_channel_event(struct event_loop *loop, int fd, struct channel *channel) {
    return event_loop_do_channel_event(loop, fd, channel, CHANNEL_OP_ADD);
}

/** 将需要从 event_dispatcher 中删除的 channel 添加到 event_loop 的队列中 */
int event_loop_remove_channel_event(struct event_loop *loop, int fd, struct channel *channel) {
    return event_loop_do_channel_event(loop, fd, channel, CHANNEL_OP_DELETE);
}

/** 将需要更新的 channel 添加到 event_loop 的队列中 */
int event_loop_update_channel_event(struct event_loop *loop, int fd, struct channel *channel) {
    return event_loop_do_channel_event(loop, fd, channel, CHANNEL_OP_UPDATE);
}


//=============================================
// 初始化与运行
//=============================================

struct event_loop *event_loop_init() {
    return event_loop_init_with_name(NULL);
}

struct event_loop *event_loop_init_with_name(const char *thread_name) {
    struct event_loop *loop = malloc(sizeof(struct event_loop));

    //线程
    pthread_mutex_init(&loop->mutex, NULL);
    pthread_cond_init(&loop->condition, NULL);
    loop->owner_thread_id = pthread_self();

    //名称
    if (thread_name == NULL) {
        loop->thread_name = "main thread";
    } else {
        loop->thread_name = thread_name;
    }

    //退出标识
    loop->quit = false;

    //映射关系 fd--channel_map
    loop->channel_map = malloc(sizeof(struct channel_map));
    channel_map_init(loop->channel_map);

    //事件分发器
#ifdef EPOLL_ENABLE
    yolanda_msgx("set epoll as event dispatcher for thread(%s)", event_loop->thread_name);
#else
    yolanda_msgx("set poll as event dispatcher for thread(%s)", loop->thread_name);
    loop->dispatcher = &poll_event_dispatcher;
#endif
    loop->dispatcher_data = loop->dispatcher->init(loop);

    //通讯通道
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, loop->socket_pair) < 0) {
        yolanda_errorx("thread(%s) create socketpair failed", loop->thread_name);
    }

    //队列
    loop->is_handling_pending_channel = false;
    loop->pending_head = loop->pending_tail = NULL;

    //创建一个通道，并设置对可读事件感兴趣，以便可以在必要的时候被唤醒。
    int notification_fd = loop->socket_pair[SOCKET_PAIR_READ];
    struct channel *notification_channel = channel_new(
            notification_fd,
            EVENT_READ,
            handle_loop_thread_wakeup,
            NULL,
            loop
    );
    //将通道添加到预处理队列中
    event_loop_add_channel_event(loop, notification_fd, notification_channel);

    return loop;
}

int event_loop_run(struct event_loop *loop) {
    assert(loop != NULL);

    //获取分发器，并检测线程
    const struct event_dispatcher *dispatcher = loop->dispatcher;
    assert_in_same_thread(loop);
    yolanda_msgx("event loop started running in thread(%s)", loop->thread_name);

    //超时时间
    struct timeval time_out;
    time_out.tv_sec = 1;

    //开启循环
    while (!loop->quit) {
        //等待 dispatcher 分发事件，并在各自 channel 的回调中处理相关事件
        dispatcher->dispatch(loop, &time_out);

        //处理完一轮事件后，看看队列中有没有新的需要注册、移除或者更新的 channel，有责进行处理
        event_loop_handle_pending_channel(loop);
    }

    yolanda_msgx("event loop exited  in thread(%s)", loop->thread_name);
    return EXIT_SUCCESS;
}

//=============================================
// 处理 dispatcher 分发的事件
//=============================================

int event_loop_active(struct event_loop *loop, int fd, int events) {
    yolanda_msgx("event_loop_active: process events(%d) for fd(%d) in thread(%s)", events, fd, loop->thread_name);
    if (fd < 0) {
        return 0;
    }

    //根据fd 从映射中找到 channel
    struct channel_map *map = loop->channel_map;

    //这个应该不会发生，检测是为了防止程序崩溃
    if (fd > map->length) {
        return -1;
    }

    struct channel *registered_channel = map->entities[fd];
    if (registered_channel == NULL) {
        yolanda_errorx("event_loop_active: fd(%d) has no related channel", fd);
        return -1;
    }

    if (registered_channel->fd != fd) {
        yolanda_errorx("event_loop_active: fd(%d) doesn't match channel(fd=)", fd, registered_channel->fd);
        exit(EXIT_FAILURE);
    }

    if ((events & EVENT_READ) != 0) {
        if (registered_channel->on_channel_readable) {
            registered_channel->on_channel_readable(registered_channel->data);
        }
    }

    if ((events & EVENT_WRITE) != 0) {
        if (registered_channel->on_channel_writeable) {
            registered_channel->on_channel_writeable(registered_channel->data);
        }
    }

    return 1;
}