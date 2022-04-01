cmake_minimum_required(VERSION 3.10.2)
project(BS_SS_2022 C)

set(CMAKE_C_STANDARD 99)

add_executable(TCPS main.c main.h keyValStore.c keyValStore.h sub.c sub.h)
