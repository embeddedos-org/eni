add_library(nia_min STATIC
    src/input.c
    src/normalizer.c
    src/filter.c
    src/mapper.c
    src/tool_bridge.c
    src/service.c
)

target_include_directories(nia_min PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(nia_min PUBLIC nia_common)
