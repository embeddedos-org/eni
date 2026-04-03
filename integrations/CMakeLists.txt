if(ENI_BUILD_ROS2)
    add_subdirectory(ros2)
endif()

if(ENI_BUILD_ONI)
    add_subdirectory(oni)
endif()

if(ENI_BUILD_EIPC_STREAM)
    add_subdirectory(eipc_streaming)
endif()
