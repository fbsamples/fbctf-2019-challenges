set(CMAKE_C_FLAGS "-Wall -Wextra -s -static -fpic -fvisibility=hidden -O3")
add_library(sha256 STATIC sha256.c)
target_include_directories (sha256 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(crypto STATIC rc4block.c)
target_include_directories (crypto PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} sha256 utils)
target_link_libraries (crypto LINK_PUBLIC sha256 utils)
