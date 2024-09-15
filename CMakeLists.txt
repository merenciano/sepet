cmake_minimum_required(VERSION 3.13.4)
set(CMAKE_C_STANDARD 99)

project(Sepet)
add_library(sepet STATIC)
target_compile_options(sepet PRIVATE -Wall)
target_include_directories(sepet PRIVATE include/sepet)

add_subdirectory(src)

add_executable(sptest src/main.c)
target_compile_options(sptest PRIVATE -std=c99 -Wall -O2)
target_include_directories(sptest PRIVATE include/)
target_link_libraries(sptest PRIVATE
    sepet
)
