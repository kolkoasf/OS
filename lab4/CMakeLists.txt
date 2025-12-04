cmake_minimum_required(VERSION 3.10)
project(PrimeTranslator)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(WIN32)
    add_definitions(-D_WIN32)
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
    set(PLATFORM_LIBS dl)
endif()

add_library(light SHARED lib_light.c)
add_library(hard SHARED lib_hard.c)

add_library(light_static STATIC lib_light.c)

add_executable(program1 program1.c)
target_link_libraries(program1 PRIVATE light_static)

add_executable(program2 program2.c)
target_link_libraries(program2 PRIVATE ${PLATFORM_LIBS})

set_target_properties(light hard
    PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

set_target_properties(program1 program2
    PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

add_dependencies(program2 light hard)
