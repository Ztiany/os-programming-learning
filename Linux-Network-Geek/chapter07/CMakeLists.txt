add_executable(unix-stream-server unix_stream_server.c)
target_link_libraries(unix-stream-server  yolanda)

add_executable(unix-stream-client unix_stream_client.c)
target_link_libraries(unix-stream-client  yolanda)

add_executable(unix-data-server unix_data_server.c)
target_link_libraries(unix-data-server  yolanda)

add_executable(unix-data-client unix_data_client.c)
target_link_libraries(unix-data-client  yolanda)
