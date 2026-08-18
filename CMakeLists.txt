cmake_minimum_required(VERSION 4.1)

project(Engine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJECT_SRC
    "src/engine.h"
    "src/engine.cpp"
)

add_executable(${PROJECT_NAME} ${PROJECT_SRC})
