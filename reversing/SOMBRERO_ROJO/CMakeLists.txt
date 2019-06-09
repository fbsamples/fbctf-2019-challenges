# CMakeLists files in this project can
# refer to the root source directory of the project
project (ctf_elf)
cmake_minimum_required(VERSION 3.10)

add_subdirectory (crypto)
add_subdirectory (utils)
add_subdirectory (flag)
add_subdirectory (test)
add_subdirectory (ctf_main)