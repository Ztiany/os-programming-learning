if (EPOLL_EXISTS)
    set(
            yolanda_lib
            log.c
    )
else ()
    set(
            yolanda_lib
            log.c
    )
endif ()

add_library(yolanda ${yolanda_lib})
target_link_libraries(yolanda pthread)

install(TARGETS yolanda DESTINATION lib)