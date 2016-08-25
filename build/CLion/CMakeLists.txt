cmake_minimum_required(VERSION 3.5)
project(httpx)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

aux_source_directory(./../../src SRC_FILES)
aux_source_directory(./../../src/events SRC_EVENTS)
add_executable(httpx ${SRC_FILES} ${SRC_EVENTS})