if (EPOLL_EXISTS)
    set(
            yolanda_lib
            log.c
            read.c
            tcp_server.c
            tcp_client.c
    )
else ()
    set(
            yolanda_lib
            log.c
            read.c
            tcp_server.c
            tcp_client.c
    )
endif ()

add_library(yolanda ${yolanda_lib})
target_link_libraries(yolanda pthread)

install(TARGETS yolanda DESTINATION lib)