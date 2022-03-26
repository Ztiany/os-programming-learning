if (EPOLL_EXISTS)
    set(
            yolanda_lib
            log.c
            read.c
    )
else ()
    set(
            yolanda_lib
            log.c
            read.c
    )
endif ()

add_library(yolanda ${yolanda_lib})
target_link_libraries(yolanda pthread)

install(TARGETS yolanda DESTINATION lib)