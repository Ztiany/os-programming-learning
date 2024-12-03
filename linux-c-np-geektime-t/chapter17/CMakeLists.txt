add_executable(17-read-closed-server read_closed_server.c)
target_link_libraries(17-read-closed-server yolanda)

add_executable(17-read-closed-client read_closed_client.c)
target_link_libraries(17-read-closed-client yolanda)

add_executable(17-write-closed-server write_closed_server.c)
target_link_libraries(17-write-closed-server yolanda)

add_executable(17-write-closed-client write_closed_client.c)
target_link_libraries(17-write-closed-client yolanda)
