add_executable(14-udp-connect-client-demo udp_connect_client_demo.c)
target_link_libraries(14-udp-connect-client-demo yolanda)

add_executable(14-udp-connect-client1 udp_connect_client1.c)
target_link_libraries(14-udp-connect-client1 yolanda)

add_executable(14-udp-connect-client2 udp_connect_client2.c)
target_link_libraries(14-udp-connect-client2 yolanda)

add_executable(14-udp-connect-server udp_connect_server.c)
target_link_libraries(14-udp-connect-server yolanda)
