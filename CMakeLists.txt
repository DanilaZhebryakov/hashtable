cmake_minimum_required(VERSION 3.0.0)
project(hashtable VERSION 0.1.0)

include(CTest)
enable_testing()

add_executable(hashtable)
target_sources(hashtable PRIVATE hashtable.cpp main_perfomance.cpp lib/file_read.cpp lib/String.cpp)

target_compile_options(hashtable PRIVATE  -O3)
target_compile_options(hashtable PRIVATE -mavx2)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
