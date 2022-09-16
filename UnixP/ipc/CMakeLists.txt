add_executable(ipc_pipe pipe.c)
target_link_libraries(ipc_pipe public_lib)

add_executable(ipc_msg_sender msg_sender.c)
target_link_libraries(ipc_msg_sender public_lib)

add_executable(ipc_msg_receiver msg_receiver.c)
target_link_libraries(ipc_msg_receiver public_lib)