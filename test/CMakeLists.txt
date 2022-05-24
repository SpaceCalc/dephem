cmake_minimum_required(VERSION 3.14)

project(test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Werror")

include_directories(..)
add_executable(test main.cpp ../dephem/dephem.h ../dephem/dephem.cpp)
add_executable(sandbox sandbox.cpp ../dephem/dephem.h ../dephem/dephem.cpp)
