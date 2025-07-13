cmake_minimum_required(VERSION 3.10)
project(led_control C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Threads REQUIRED)
find_library(PIGPIO_LIB pigpio)

add_executable(led_control led_control.c)
target_link_libraries(led_control PRIVATE Threads::Threads ${PIGPIO_LIB} rt)
