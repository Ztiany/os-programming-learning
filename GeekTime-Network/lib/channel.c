#include <stdlib.h>
#include "channel.h"
#include "event_loop.h"

struct channel *channel_new(
        int fd,
        int events,
        event_read_callback read_callback,
        event_write_callback write_callback,
        void *data
) {
    struct channel *channel = malloc(sizeof(struct channel));
    if (channel == NULL) {
        return NULL;
    }

    channel->fd = fd;
    channel->events = events;
    channel->on_channel_readable = read_callback;
    channel->on_channel_writeable = write_callback;
    channel->data = data;

    return channel;
}

void channel_free(struct channel *channel) {
    free(channel);
}

bool channel_event_is_readable(struct channel *channel) {
    if (channel == NULL) {
        return false;
    }
    if ((channel->events & EVENT_READ) == 0) {
        return false;
    } else {
        return true;
    }
}

bool channel_event_is_writeable(struct channel *channel) {
    if (channel == NULL) {
        return false;
    }
    if ((channel->events & EVENT_WRITE) == 0) {
        return false;
    } else {
        return true;
    }
}

void channel_event_enable_write(struct channel *channel) {
    struct event_loop *loop = (struct event_loop *) channel->data;
    channel->events = channel->events | EVENT_WRITE;
    event_loop_update_channel_event(loop, channel->fd, channel);
}

void channel_event_disable_write(struct channel *channel) {
    struct event_loop *loop = (struct event_loop *) channel->data;
    channel->events = channel->events & (~EVENT_WRITE);
    event_loop_update_channel_event(loop, channel->fd, channel);
}
