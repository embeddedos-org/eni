add_library(nia_platform STATIC)

target_include_directories(nia_platform PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(nia_platform PUBLIC nia_common)

if(NIA_PLATFORM_LINUX)
    target_sources(nia_platform PRIVATE linux/platform_linux.c)
endif()

if(NIA_PLATFORM_WINDOWS)
    target_sources(nia_platform PRIVATE windows/platform_windows.c)
endif()

if(NIA_PLATFORM_FORGEOS)
    target_sources(nia_platform PRIVATE forgeos/platform_forgeos.c)
    target_compile_definitions(nia_platform PRIVATE NIA_PLATFORM_FORGEOS_ENABLED)
endif()
