#include "channel.h"
#include <stdlib.h>

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
    //TODO：update to event_loop
    channel->events |= EVENT_WRITE;
}

void channel_event_disable_write(struct channel *channel) {
    //TODO：update to event_loop
    channel->events = channel->events & ~EVENT_WRITE;
}
