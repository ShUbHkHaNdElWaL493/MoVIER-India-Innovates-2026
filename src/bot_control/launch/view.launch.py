#   Shubh Khandelwal

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare

def generate_launch_description():

    mode = LaunchConfiguration("mode")
    rviz_config = LaunchConfiguration("rviz_config")

    robot_node = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(PathJoinSubstitution([FindPackageShare("bot_control"), "launch", "spawn.launch.py"])),
        launch_arguments = {
            "mode" : mode
        }.items()
    )

    rviz_node = Node(
        package = "rviz2",
        executable = "rviz2",
        name = "rviz2",
        output = "log",
        arguments = ["-d", rviz_config]
    )

    return LaunchDescription([
        DeclareLaunchArgument(
            "mode",
            default_value = "gz",
            description = "Robot mode.",
            choices = [
                "gz",
                "hw"
            ]
        ),
        DeclareLaunchArgument(
            "rviz_config",
            default_value = PathJoinSubstitution([FindPackageShare("bot_description"), "rviz", "view.rviz"]),
            description = "Rviz config file (absolute path) to use when launching rviz."
        ),
        robot_node,
        rviz_node
    ])