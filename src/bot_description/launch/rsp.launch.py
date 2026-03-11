#   Shubh Khandelwal

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, OpaqueFunction
from launch.substitutions import Command, FindExecutable, LaunchConfiguration, PathJoinSubstitution
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare

def launch_setup(context):

    mode = LaunchConfiguration("mode")
    description_file = LaunchConfiguration("description_file")

    robot_description = Command([
        PathJoinSubstitution([FindExecutable(name = "xacro")]),
        " ",
        description_file,
        " ",
        "mode:=",
        mode
    ])

    if mode.perform(context) == "gz":
        robot_state_publisher_node = Node(
            package = "robot_state_publisher",
            executable = "robot_state_publisher",
            output = "both",
            parameters = [{
                "use_sim_time" : True,
                "robot_description" : robot_description
            }]
        )

    if mode.perform(context) == "hw":
        robot_state_publisher_node = Node(
            package = "robot_state_publisher",
            executable = "robot_state_publisher",
            output = "both",
            parameters = [{
                "use_sim_time" : False,
                "robot_description" : robot_description
            }]
        )
    
    return [robot_state_publisher_node]

def generate_launch_description():

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
            "description_file",
            default_value = PathJoinSubstitution([FindPackageShare("bot_description"), "models", "robot.urdf.xacro"]),
            description = "URDF/XACRO description file (absolute path) with the robot."
        )
    ] + [OpaqueFunction(function = launch_setup)])