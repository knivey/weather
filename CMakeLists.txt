cmake_minimum_required(VERSION 3.0)

project(weather)

add_executable(weather main.cpp)

install(TARGETS weather RUNTIME DESTINATION bin)
