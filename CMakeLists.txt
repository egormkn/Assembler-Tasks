cmake_minimum_required(VERSION 3.7)
project(Assembler)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

set(MEMCPY_SOURCE_FILES memcpy.cpp memcpy.h)
add_executable(memcpy ${MEMCPY_SOURCE_FILES})

set(WORDCOUNT_SOURCE_FILES wordcount.cpp wordcount.h)
add_executable(wordcount ${WORDCOUNT_SOURCE_FILES})
target_compile_options(wordcount PUBLIC -mssse3)
