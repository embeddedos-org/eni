cmake_minimum_required(VERSION 3.14)
project(eni_sim C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(eni_sim STATIC
    src/sim_environment.c
    src/sim_eeg_generator.c
    src/sim_bci_generator.c
)

target_include_directories(eni_sim PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(eni_sim PRIVATE m)

# --- Tests ----------------------------------------------------------------
option(ENI_SIM_BUILD_TESTS "Build simulation tests" OFF)

if(ENI_SIM_BUILD_TESTS)
    enable_testing()

    add_executable(test_sim_eeg tests/test_sim_eeg.c)
    target_link_libraries(test_sim_eeg PRIVATE eni_sim m)
    add_test(NAME sim_eeg COMMAND test_sim_eeg)

    add_executable(test_sim_bci tests/test_sim_bci.c)
    target_link_libraries(test_sim_bci PRIVATE eni_sim m)
    add_test(NAME sim_bci COMMAND test_sim_bci)
endif()
