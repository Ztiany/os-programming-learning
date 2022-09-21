add_executable(ipc_pipe pipe.c)
target_link_libraries(ipc_pipe public_lib)

add_executable(ipc_msg_sender sysv_msg_sender.c)
target_link_libraries(ipc_msg_sender public_lib)

add_executable(ipc_msg_receiver sysv_msg_receiver.c)
target_link_libraries(ipc_msg_receiver public_lib)

add_executable(ipc_sem_s_sample sysv_s_sem_sample.cpp)
target_link_libraries(ipc_sem_s_sample public_lib)

add_executable(ipc_sem_sample sysv_sem_sample.cpp)
target_link_libraries(ipc_sem_sample public_lib)

add_executable(ipc_sem_init sysv_sm_init.c)
target_link_libraries(ipc_sem_init public_lib)