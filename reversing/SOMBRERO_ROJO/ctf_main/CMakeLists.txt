project(reallybad C)

set(CMAKE_C_FLAGS "-Wall -Wextra -s -static -fpic -fvisibility=hidden -O3")

add_executable (reallybad main.c)
target_include_directories(reallybad PUBLIC utils crypto)
target_link_libraries (reallybad LINK_PUBLIC utils crypto)

