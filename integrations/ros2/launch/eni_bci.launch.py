"""eNI ROS 2 Launch File — BCI node with configurable parameters."""

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration


def generate_launch_description():
    return LaunchDescription([
        DeclareLaunchArgument("provider", default_value="simulator", description="Neural input provider"),
        DeclareLaunchArgument("decoder", default_value="energy", description="Decoder type"),
        DeclareLaunchArgument("feedback", default_value="false", description="Enable feedback loop"),
        DeclareLaunchArgument("rate", default_value="30", description="Publish rate (Hz)"),
    ])
