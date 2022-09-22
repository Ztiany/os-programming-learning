add_executable(47.3.sems_monitor 47.3.sems_monitor_.c)
target_link_libraries(47.3.sems_monitor libtlpi)

add_executable(47.6.sems_good_init 47.6.sems_good_init.c)
target_link_libraries(47.6.sems_good_init libtlpi)

add_executable(47.9.binary_sems_sample 47.9.binary_sems.c 47.9.binary_sems_sample.c)
target_link_libraries(47.9.binary_sems_sample libtlpi)
