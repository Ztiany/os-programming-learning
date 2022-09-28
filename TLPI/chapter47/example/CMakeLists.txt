add_executable(47.03.sems_monitor 47.03.sems_monitor_.c)
target_link_libraries(47.03.sems_monitor libtlpi)

add_executable(47.06.sems_good_init 47.06.sems_good_init.c)
target_link_libraries(47.06.sems_good_init libtlpi)

add_executable(47.09.binary_sems_sample 47.09.binary_sems.c 47.09.binary_sems_sample.c)
target_link_libraries(47.09.binary_sems_sample libtlpi)
