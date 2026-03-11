#   Shubh Khandelwal

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare


def generate_launch_description():

    rviz_config = LaunchConfiguration("rviz_config")

    robot_state_publisher_node = IncludeLaunchDescription(PythonLaunchDescriptionSource(
        PathJoinSubstitution([FindPackageShare("bot_description"), "launch", "rsp.launch.py"])
    ))

    rviz_node = Node(
        package = "rviz2",
        executable = "rviz2",
        name = "rviz2",
        output = "log",
        arguments = ["-d", rviz_config]
    )

    joint_state_publisher_node = Node(
        package = "joint_state_publisher",
        executable = "joint_state_publisher",
        name = "joint_state_publisher",
        output = "log"
    )

    return LaunchDescription([
        DeclareLaunchArgument(
            "rviz_config",
            default_value = PathJoinSubstitution([FindPackageShare("bot_description"), "rviz", "view.rviz"]),
            description = "Rviz config file (absolute path) to use when launching rviz."
        ),
        robot_state_publisher_node,
        rviz_node,
        joint_state_publisher_node
    ])