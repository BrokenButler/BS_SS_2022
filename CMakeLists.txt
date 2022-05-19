cmake_minimum_required(VERSION 3.10.2)
project(BS_SS_2022 C)

set(CMAKE_C_STANDARD 11)

add_executable(TCPS
        keyValStore.h
        keyValStore.c
        sub.h
        sub.c
        main.h
        main.c)