cmake_minimum_required(VERSION 3.14)
project(eni_wireless_bci C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(eni_wireless_bci STATIC
    wireless_bci.c
)

target_include_directories(eni_wireless_bci PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../common/hal/include
)

target_link_libraries(eni_wireless_bci PRIVATE m)
