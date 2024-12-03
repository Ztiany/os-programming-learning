add_executable(io_open_file io_open_file.c)
target_link_libraries(io_open_file alien)

add_executable(io_read_write io_read_write.c)
target_link_libraries(io_read_write alien)

add_executable(io_lseek io_lseek.c)
target_link_libraries(io_lseek alien)

add_executable(io_fcntl io_fcntl.c)
target_link_libraries(io_fcntl alien)

add_executable(io_ioctl io_ioctl.c)
target_link_libraries(io_ioctl alien)

add_executable(io_list_file io_list_file.c)
target_link_libraries(io_list_file alien)