cmake_minimum_required(VERSION 3.10)
project(DynamicLibraries)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(WIN32)
    add_definitions(-D_WIN32)
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
    set(PLATFORM_LIBS dl)
endif()

add_library(primes SHARED lib_primes.c)
add_library(translation SHARED lib_translation.c)

add_executable(program1 program1.c)
target_link_libraries(program1 PRIVATE primes translation)

add_executable(program2 program2.c)
target_link_libraries(program2 PRIVATE ${PLATFORM_LIBS})

set_target_properties(primes translation
    PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

set_target_properties(program1 program2
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)