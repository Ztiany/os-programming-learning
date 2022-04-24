add_executable(16-stream-client stream_client.c)
target_link_libraries(16-stream-client yolanda)

add_executable(16-stream-server stream_server.c)
target_link_libraries(16-stream-server yolanda)
