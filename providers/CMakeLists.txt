add_library(eni_providers STATIC
    simulator/simulator.c
    generic/generic.c
)

target_include_directories(eni_providers PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/simulator
    ${CMAKE_CURRENT_SOURCE_DIR}/generic
)

target_link_libraries(eni_providers PUBLIC eni_common)
