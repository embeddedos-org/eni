cmake_minimum_required(VERSION 3.16)
project(NIA
    VERSION 0.1.0
    DESCRIPTION "Neural Interface Adapter — BCI/sensor input integration for ForgeOS AI Layer"
    LANGUAGES C
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

option(NIA_BUILD_MIN       "Build NIA-Min lightweight runtime"       ON)
option(NIA_BUILD_FRAMEWORK "Build NIA-Framework industrial platform" ON)
option(NIA_BUILD_CLI       "Build NIA CLI tool"                      ON)
option(NIA_BUILD_TESTS     "Build unit tests"                        OFF)

option(NIA_PLATFORM_FORGEOS   "Enable ForgeOS platform adapter"   OFF)
option(NIA_PLATFORM_LINUX     "Enable Linux platform adapter"     ON)
option(NIA_PLATFORM_WINDOWS   "Enable Windows platform adapter"   OFF)
option(NIA_PLATFORM_CONTAINER "Enable container platform adapter" OFF)

if(WIN32)
    set(NIA_PLATFORM_LINUX OFF)
    set(NIA_PLATFORM_WINDOWS ON)
endif()

# Common library (shared contracts + utilities)
add_subdirectory(common)

# Platform adapters
add_subdirectory(platform)

# Providers (simulator, generic decoder)
add_subdirectory(providers)

if(NIA_BUILD_MIN)
    add_subdirectory(min)
endif()

if(NIA_BUILD_FRAMEWORK)
    add_subdirectory(framework)
endif()

if(NIA_BUILD_CLI)
    add_subdirectory(cli)
endif()

if(NIA_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

message(STATUS "--------------------------------------------")
message(STATUS "NIA v${PROJECT_VERSION}")
message(STATUS "  NIA-Min:       ${NIA_BUILD_MIN}")
message(STATUS "  NIA-Framework: ${NIA_BUILD_FRAMEWORK}")
message(STATUS "  CLI:           ${NIA_BUILD_CLI}")
message(STATUS "  Platform:      forgeos=${NIA_PLATFORM_FORGEOS} linux=${NIA_PLATFORM_LINUX} win=${NIA_PLATFORM_WINDOWS} container=${NIA_PLATFORM_CONTAINER}")
message(STATUS "--------------------------------------------")
