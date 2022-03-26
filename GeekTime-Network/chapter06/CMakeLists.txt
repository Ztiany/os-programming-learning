add_executable(udp-server udp_server.c)
target_link_libraries(udp-server  yolanda)

add_executable(udp-client udp_client.c)
target_link_libraries(udp-client  yolanda)