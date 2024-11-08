add_executable(tcp-server tcp_server.c)
target_link_libraries(tcp-server  yolanda)

add_executable(tcp-client tcp_client.c)
target_link_libraries(tcp-client  yolanda)