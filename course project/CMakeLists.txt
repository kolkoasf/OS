cmake_minimum_required(VERSION 3.10)
project(BattleshipGame C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(COMMON_SOURCES
    game_logic.c
    stats.c
    sync.c
    os_linux.c
)

set(SERVER_SOURCES
    ${COMMON_SOURCES}
    server.c
    battleship_server.c
)

set(CLIENT_SOURCES
    ${COMMON_SOURCES}
    server.c
    battleship_client.c
)

find_package(Threads REQUIRED)

add_executable(battleship_server ${SERVER_SOURCES})
target_link_libraries(battleship_server PRIVATE Threads::Threads)
target_compile_options(battleship_server PRIVATE -DOS_LINUX=1)

add_executable(battleship_client ${CLIENT_SOURCES})
target_link_libraries(battleship_client PRIVATE Threads::Threads)
target_compile_options(battleship_client PRIVATE -DOS_LINUX=1)

target_include_directories(battleship_server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(battleship_client PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

install(TARGETS battleship_server battleship_client
        RUNTIME DESTINATION bin)
