cmake_minimum_required(VERSION 3.5)

project(cpp-shared LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(cpp-shared main.cpp)
add_executable(factory factory.cpp)
add_executable(observer observer.cpp)
