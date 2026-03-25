add_library(nia_providers STATIC
    simulator/simulator.c
    generic/generic.c
)

target_include_directories(nia_providers PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/simulator
    ${CMAKE_CURRENT_SOURCE_DIR}/generic
)

target_link_libraries(nia_providers PUBLIC nia_common)
