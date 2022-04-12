add_executable(11-grace-client grace_client.c)
target_link_libraries(11-grace-client  yolanda)

add_executable(11-grace-server grace_server.c)
target_link_libraries(11-grace-server yolanda)
