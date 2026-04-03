add_library(eni_eipc_stream STATIC src/eipc_stream.c)
target_include_directories(eni_eipc_stream PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(eni_eipc_stream PUBLIC eni_common)
