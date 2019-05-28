cmake_minimum_required(VERSION 3.9)

project(generator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# https://clang.llvm.org/docs/DiagnosticsReference.html
add_compile_options(-Wall -Wextra -Wpedantic -g -fno-omit-frame-pointer)

add_subdirectory(twist)
add_subdirectory(solution)

