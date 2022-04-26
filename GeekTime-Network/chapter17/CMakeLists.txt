add_executable(17-write-to-fin-server write_to_fin_server.c)
target_link_libraries(17-write-to-fin-server yolanda)

add_executable(17-write-to-fin-client write_to_fin_client.c)
target_link_libraries(17-write-to-fin-client yolanda)
