add_executable(26-threads-server loop_echo.c threads_server.c)
target_link_libraries(26-threads-server yolanda)

add_executable(26-thread-pool-server loop_echo.c thread_pool_server.c)
target_link_libraries(26-thread-pool-server yolanda)
