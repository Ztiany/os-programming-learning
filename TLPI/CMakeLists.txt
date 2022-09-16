# 指定 CMake 版本
cmake_minimum_required(VERSION 3.10)

project(TLPI)

set(CMAKE_C_STANDARD 99)

include_directories(lib)
add_subdirectory(lib)

file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/chapter04/example/*.c")
#file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/chapter04/exercise/*.c")

foreach(file ${files})
    get_filename_component(name ${file} NAME)
    add_executable(${name} ${file})
    target_link_libraries(${name}  libtlpi)
endforeach()