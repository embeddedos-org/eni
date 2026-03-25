add_executable(eni_test_common test_common.c)
target_link_libraries(eni_test_common PRIVATE eni_common)
add_test(NAME eni_common_tests COMMAND eni_test_common)

add_executable(eni_test_min test_min.c)
target_link_libraries(eni_test_min PRIVATE eni_common eni_min)
add_test(NAME eni_min_tests COMMAND eni_test_min)

add_executable(eni_test_framework test_framework.c)
target_link_libraries(eni_test_framework PRIVATE eni_common eni_framework)
add_test(NAME eni_framework_tests COMMAND eni_test_framework)

add_executable(eni_test_providers test_providers.c)
target_link_libraries(eni_test_providers PRIVATE eni_common eni_providers)
add_test(NAME eni_provider_tests COMMAND eni_test_providers)

# --- test_event: Event system ---
add_executable(eni_test_event test_event.c)
target_link_libraries(eni_test_event PRIVATE eni_common)
add_test(NAME eni_event_tests COMMAND eni_test_event)

# --- test_policy: Policy engine ---
add_executable(eni_test_policy test_policy.c)
target_link_libraries(eni_test_policy PRIVATE eni_common)
add_test(NAME eni_policy_tests COMMAND eni_test_policy)

# --- test_neuralink: Neuralink provider ---
add_executable(eni_test_neuralink test_neuralink.c)
target_link_libraries(eni_test_neuralink PRIVATE eni_common eni_providers)
add_test(NAME eni_neuralink_tests COMMAND eni_test_neuralink)

# --- test_filter: Min filter ---
add_executable(eni_test_filter test_filter.c)
target_link_libraries(eni_test_filter PRIVATE eni_common eni_min)
add_test(NAME eni_filter_tests COMMAND eni_test_filter)

# --- test_normalizer: Min normalizer ---
add_executable(eni_test_normalizer test_normalizer.c)
target_link_libraries(eni_test_normalizer PRIVATE eni_common eni_min)
add_test(NAME eni_normalizer_tests COMMAND eni_test_normalizer)
