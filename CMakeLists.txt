cmake_minimum_required(VERSION 3.5)

project(BPlusTree VERSION 0.1 LANGUAGES CXX)

# set the C++ standard to C++11
set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES ${PROJECT_SOURCE_DIR}/src/main.cpp)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

# target_include_directories must be behind add_executable
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/include)
