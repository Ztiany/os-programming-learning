add_executable(12-ping-client ping_client.c)
target_link_libraries(12-ping-client  yolanda)

add_executable(12-ping-server ping_server.c)
target_link_libraries(12-ping-server yolanda)
