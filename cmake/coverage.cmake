# Coverage collection module for gcov/lcov
option(ENI_ENABLE_COVERAGE "Enable code coverage collection" OFF)

if(ENI_ENABLE_COVERAGE)
    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        add_compile_options(--coverage -fprofile-arcs -ftest-coverage)
        add_link_options(--coverage)

        find_program(LCOV lcov)
        find_program(GENHTML genhtml)

        if(LCOV AND GENHTML)
            add_custom_target(coverage
                COMMAND ${LCOV} --capture --directory ${CMAKE_BINARY_DIR} --output-file coverage.info --rc lcov_branch_coverage=1
                COMMAND ${LCOV} --remove coverage.info '/usr/*' '*/tests/*' --output-file coverage_filtered.info --rc lcov_branch_coverage=1
                COMMAND ${GENHTML} coverage_filtered.info --output-directory ${CMAKE_BINARY_DIR}/coverage_report --branch-coverage
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                COMMENT "Generating code coverage report..."
            )

            add_custom_target(coverage-reset
                COMMAND ${LCOV} --zerocounters --directory ${CMAKE_BINARY_DIR}
                COMMENT "Resetting coverage counters..."
            )
        else()
            message(WARNING "lcov/genhtml not found — coverage reports disabled")
        endif()
    else()
        message(WARNING "Coverage only supported with GCC/Clang")
    endif()
endif()
