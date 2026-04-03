cmake_minimum_required(VERSION 3.20)
project(eni_ros2_node C)

find_package(rclc QUIET)
if(NOT rclc_FOUND)
    message(WARNING "rclc not found — ROS 2 integration will be stub-only")
endif()

add_library(eni_ros2 STATIC src/eni_ros2_node.c)
target_include_directories(eni_ros2 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(eni_ros2 PUBLIC eni_common)

if(rclc_FOUND)
    target_link_libraries(eni_ros2 PUBLIC rclc::rclc)
    target_compile_definitions(eni_ros2 PUBLIC ENI_HAS_ROS2)
endif()
