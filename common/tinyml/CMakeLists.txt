add_library(eni_tinyml STATIC src/tflite_micro_bridge.c)
target_include_directories(eni_tinyml PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(eni_tinyml PUBLIC eni_common)
