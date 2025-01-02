# CMakeLists for chessTest
# 10-11-2023 by madpl

cmake_minimum_required(VERSION 3.22)
project(stockTest LANGUAGES CXX)

# standard C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SOURCES ./source/*)
add_definitions(-Os -march=native)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
	set(SFML_DIR "C:/Progs/SFML2.5.1SEH/lib/cmake/SFML"
		CACHE PATH "Path to SFML CMake configuration")
endif()

find_package(SFML 2.5 COMPONENTS system window graphics REQUIRED)

set(EXECUTABLE_OUTPUT_PATH  ${CMAKE_SOURCE_DIR})
add_executable(${CMAKE_PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} sfml-system sfml-window sfml-graphics)
