cmake_minimum_required(VERSION 3.5)
project(BPlusTree VERSION 0.1 LANGUAGES CXX)

include_directories(${PROJECT_SOURCE_DIR}/include)
add_executable(example ${PROJECT_SOURCE_DIR}/src/example.cpp)
