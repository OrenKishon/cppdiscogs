cmake_minimum_required(VERSION 3.10)
project(cppdiscogs)

set(CMAKE_CXX_STANDARD 20)

find_package(CURL REQUIRED)

include_directories(${CURL_INCLUDE_DIRS})

add_executable(cppdiscogs main.cpp client.cpp collection.cpp)

target_link_libraries(cppdiscogs ${CURL_LIBRARIES})
