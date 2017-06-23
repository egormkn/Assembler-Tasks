cmake_minimum_required(VERSION 3.7)
project(Assembler)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

set(MEMCPY_SOURCE_FILES memcpy.cpp memcpy.h)
add_executable(memcpy ${MEMCPY_SOURCE_FILES})
target_compile_options(memcpy PUBLIC -Wall -Werror -Wextra)

set(WORDCOUNT_SOURCE_FILES wordcount.cpp wordcount.h)
add_executable(wordcount ${WORDCOUNT_SOURCE_FILES})
target_compile_options(wordcount PUBLIC -Wall -Werror -Wextra -mssse3)

set(TRAMPOLINE_SOURCE_FILES trampoline.cpp trampoline.h trampoline_args.h)
add_executable(trampoline ${TRAMPOLINE_SOURCE_FILES})