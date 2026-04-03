# Static analysis integration (cppcheck + clang-tidy)
option(ENI_ENABLE_CPPCHECK   "Enable cppcheck static analysis"   OFF)
option(ENI_ENABLE_CLANG_TIDY "Enable clang-tidy static analysis" OFF)

if(ENI_ENABLE_CPPCHECK)
    find_program(CPPCHECK cppcheck)
    if(CPPCHECK)
        set(CMAKE_C_CPPCHECK
            ${CPPCHECK}
            --enable=warning,performance,portability
            --suppress=missingIncludeSystem
            --inline-suppr
            --std=c11
            --quiet
        )
        message(STATUS "cppcheck enabled: ${CPPCHECK}")
    else()
        message(WARNING "cppcheck not found — static analysis disabled")
    endif()
endif()

if(ENI_ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY clang-tidy)
    if(CLANG_TIDY)
        set(CMAKE_C_CLANG_TIDY
            ${CLANG_TIDY}
            -checks=-*,bugprone-*,cert-*,clang-analyzer-*,performance-*,portability-*
            --warnings-as-errors=bugprone-*,cert-*
        )
        message(STATUS "clang-tidy enabled: ${CLANG_TIDY}")
    else()
        message(WARNING "clang-tidy not found — static analysis disabled")
    endif()
endif()
