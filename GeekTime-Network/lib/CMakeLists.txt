if (EPOLL_EXISTS)
    set(
            yolanda_lib
            log.c
            read.c
            write.c
            tcp_server.c
            tcp_client.c
            chars.c
    )
else ()
    set(
            yolanda_lib
            log.c
            read.c
            write.c
            tcp_server.c
            tcp_client.c
            chars.h
    )
endif ()

add_library(yolanda ${yolanda_lib})
target_link_libraries(yolanda pthread)

install(TARGETS yolanda DESTINATION lib)