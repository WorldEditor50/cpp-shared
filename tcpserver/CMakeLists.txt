cmake_minimum_required(VERSION 3.16)
project(sample VERSION 0.1)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(LIBS "pthread")

include_directories(${SRC_DIR})

file(GLOB SERVER_FILES 
    ${SRC_DIR}/socket.hpp
    ${SRC_DIR}/tcpserver.h
    ${SRC_DIR}/tcpserver.cpp
    ${SRC_DIR}/serverhandler.h
    ${SRC_DIR}/serverhandler.cpp
    ${SRC_DIR}/serverd.cpp
)
add_executable(serverd ${SERVER_FILES})
target_link_libraries(serverd PRIVATE ${LIBS})

file(GLOB CLIENT_FILES 
    ${SRC_DIR}/socket.hpp
    ${SRC_DIR}/tcpclient.h
    ${SRC_DIR}/tcpclient.cpp
    ${SRC_DIR}/clienthandler.h
    ${SRC_DIR}/clienthandler.cpp
    ${SRC_DIR}/clientd.cpp
)
add_executable(clientd ${CLIENT_FILES})
target_link_libraries(clientd PRIVATE ${LIBS})
