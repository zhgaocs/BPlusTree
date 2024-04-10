cmake_minimum_required(VERSION 3.14)

project(BPlusTree)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
    test/main.cpp
)

add_executable(bptree-test ${SOURCE_FILES})

target_include_directories(bptree-test PRIVATE ${PROJECT_SOURCE_DIR}/include)