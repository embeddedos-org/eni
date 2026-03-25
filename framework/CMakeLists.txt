add_library(nia_framework STATIC
    src/provider_manager.c
    src/stream_bus.c
    src/router.c
    src/orchestrator.c
    src/connectors.c
    src/observability.c
    src/service.c
)

target_include_directories(nia_framework PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(nia_framework PUBLIC nia_common)
