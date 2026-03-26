add_library(eni_common STATIC
    src/types.c
    src/event.c
    src/log.c
    src/config.c
    src/policy.c
    src/tool_call.c
    src/provider.c
)

target_include_directories(eni_common PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
