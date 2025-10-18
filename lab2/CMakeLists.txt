cmake_minimum_required(VERSION 3.10)
project(lab2_os)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(WIN32)
    message(STATUS "Configuring for Windows")
else()
    message(STATUS "Configuring for Linux/Unix")
    set(OS_SOURCES triangle_finder_linux.c thread_linux.c mutex_linux.c)
    set(PLATFORM_LIBS pthread m)
endif()

add_executable(task
    task.c
    ${OS_SOURCES}
)

target_include_directories(task PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(task PRIVATE ${PLATFORM_LIBS})

