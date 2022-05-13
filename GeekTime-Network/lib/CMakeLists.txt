set(
        yolanda_lib
        log.c
        read.c
        write.c
        tcp_server.c
        tcp_client.c
        chars.c
        acceptor.c
        buffer.c
        channel.c
        channel_map.c
        event_loop.c
        event_loop_thread.c
        tcp_connection.c
        thread_pool.c
        utils.c
        event_dispatcher_poll.c
        event_dispatcher_epoll.c
)

# Clion 的 bug，识别不到下面方式添加的 event_dispatcher_epoll。
#if (EPOLL_EXISTS)
#    MESSAGE("include epoll")
#    list(APPEND ${yolanda_lib} event_dispatcher_epoll.c)
#endif ()

add_library(yolanda ${yolanda_lib})
target_link_libraries(yolanda pthread)

install(TARGETS yolanda DESTINATION lib)