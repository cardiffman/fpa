cmake_minimum_required(VERSION 3.0)
project(tim,cxx)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 14)

add_executable(fpa fpa.cpp fpnode.cpp fptoken.cpp fpsuper.cpp)
target_compile_options(fpa PRIVATE "-Wall")
