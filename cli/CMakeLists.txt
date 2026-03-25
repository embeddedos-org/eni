add_executable(nia main.c)

target_link_libraries(nia PRIVATE
    nia_common
    nia_min
    nia_framework
    nia_providers
    nia_platform
)
