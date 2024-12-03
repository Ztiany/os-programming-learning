# 使用命名管道
add_executable(ipc_named_pipe named_pipe.c)
target_link_libraries(ipc_named_pipe alien)

# 使用 System V 消息队列
add_executable(ipc_msg_sender sysv_msg_sender.c)
target_link_libraries(ipc_msg_sender alien)
add_executable(ipc_msg_receiver sysv_msg_receiver.c)
target_link_libraries(ipc_msg_receiver alien)

#  演示 System V 信号量
add_executable(ipc_sem_s_sample sysv_s_sem_sample.cpp)
target_link_libraries(ipc_sem_s_sample alien)
add_executable(ipc_sem_sample sysv_sem_sample.cpp)
target_link_libraries(ipc_sem_sample alien)

#  演示 System V 共享内存
add_executable(ipc_shm_sample sysv_shm_sample.c)
target_link_libraries(ipc_shm_sample alien)
