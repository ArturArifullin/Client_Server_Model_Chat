cmake_minimum_required(VERSION 3.20)
project(serv)

set(CMAKE_CXX_STANDARD 11)

add_executable(serv main.cpp profile.h profile.cpp server.h server.cpp)