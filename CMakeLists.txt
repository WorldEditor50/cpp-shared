cmake_minimum_required(VERSION 3.5)

project(cpp-shared LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#src
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/test)
file(GLOB SRC_FILES
    ${SRC_DIR}/*.h
    ${SRC_DIR}/*.hpp
    ${SRC_DIR}/*.cpp)

add_executable(cpp-shared ${SRC_FILES} ${TEST_DIR}/main.cpp)
add_executable(factory ${TEST_DIR}/factory.cpp)
add_executable(observer ${TEST_DIR}/observer.cpp)
