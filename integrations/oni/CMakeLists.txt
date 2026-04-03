add_library(eni_oni STATIC src/oni_compliance.c)
target_include_directories(eni_oni PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(eni_oni PUBLIC eni_common)
