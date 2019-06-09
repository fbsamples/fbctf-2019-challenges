project(encrypt_file C)

set(random_key "EC6PITweypl31NyBtmiahESsyEddxVmj")
add_definitions(-Drand_key="${random_key}")

set(half_kek1 "RkxBR3s3aDQ3X1c0c243X1MwX0g0UmR9")
add_definitions(-DHALF_KEY_ENC_KEY1="${half_kek1}")

set(half_key2 "fb{7h47_W4sn7_S0_H4Rd}")
add_definitions(-DHALF_KEY_ENC_KEY2="${half_key2}")

set(GCC_DEBUG_SYMBOLS "-ggdb")
add_definitions(${GCC_DEBUG_SYMBOLS})

add_executable (encrypt_file main.c)
target_include_directories(encrypt_file PUBLIC utils crypto)
target_link_libraries (encrypt_file LINK_PUBLIC utils crypto)