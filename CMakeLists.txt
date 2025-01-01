# CMakeLists for chessTest
# 10-11-2023 by madpl

cmake_minimum_required(VERSION 3.31)
project(stockTest LANGUAGES CXX)

file(GLOB_RECURSE SOURCES ./source/*)
add_definitions(-Os -march=native -std=c++23)

find_package(SFML 2.5 COMPONENTS system window graphics REQUIRED)

set(EXECUTABLE_OUTPUT_PATH  ${CMAKE_SOURCE_DIR})
add_executable(${CMAKE_PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} sfml-system sfml-window sfml-graphics)
