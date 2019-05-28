cmake_minimum_required(VERSION 3.9)

project(generator)

include(cmake/Logging.cmake)
include(cmake/Platform.cmake)
include(cmake/CheckCompiler.cmake)
include(cmake/CompileOptions.cmake)

add_subdirectory(twist)
add_subdirectory(solution)

