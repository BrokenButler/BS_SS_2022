cmake_minimum_required(VERSION 3.10.2)
project(BS_SS_2022 C)

set(CMAKE_C_STANDARD 11)

include_directories(commandHandling)
include_directories(keyValueStore)
include_directories(semaphores)
include_directories(server)
include_directories(socket)

add_executable(TCPS
        commandHandling/commandHandler.h
        keyValueStore.h
        server/server.h
        socket/socketHandler.h
        commandHandling/commandHandler.c
        keyValueStore.c
        server/server.c
        socket/socketHandler.c
        main.c)