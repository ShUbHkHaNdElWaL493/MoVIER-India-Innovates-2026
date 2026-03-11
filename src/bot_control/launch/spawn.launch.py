from ament_index_python import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare
import os

def generate_launch_description():

    mode = LaunchConfiguration("mode")
    world_file = LaunchConfiguration("world_file")
    
    use_sim_time = LaunchConfiguration('use_sim_time')
    sim_time_arg = DeclareLaunchArgument(
        'use_sim_time', 
        default_value='true',
        description='Use simulation (Gazebo) clock if true'
    )

    rsp_node = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(PathJoinSubstitution([FindPackageShare("bot_description"), "launch", "rsp.launch.py"])),
        launch_arguments = {
            "description_file" : PathJoinSubstitution([FindPackageShare("bot_control"), "models", "robot.urdf.xacro"]),
            "mode" : mode
        }.items()
    )

    gz_node = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(PathJoinSubstitution([FindPackageShare("ros_gz_sim"), "launch", "gz_sim.launch.py"])),
        launch_arguments = {
            "gz_args": ["-r -v4 ", world_file],
            # --- FIX 2: ADD THIS LINE BACK ---
            "use_sim_time": use_sim_time
        }.items(),
    )

    ros_gz_bridge_node = Node(
        package = "ros_gz_bridge",
        executable = "parameter_bridge",
        arguments = [
            "--ros-args", "-p",
            f"config_file:={os.path.join(get_package_share_directory('bot_control'), 'config', 'bridge.yaml')}"
        ],
        output = "screen",
        # Add use_sim_time to all ROS nodes
        parameters=[
            {
                'use_sim_time': use_sim_time
            }
        ]
    )

    ros_image_bridge_node = Node(
        package = 'ros_gz_image',
        executable = 'image_bridge',
        arguments = ['/top_camera/image_raw'],
        output = 'screen',
        parameters=[
            {
                'use_sim_time': use_sim_time
            }
        ]
    )

    entity_spawner_node = Node(
        package = "ros_gz_sim",
        executable = "create",
        output = "screen",
        arguments = [
            "-name",
            "robot",
            "-topic",
            "robot_description",
            "-z", "0.15",
            "-allow_renaming",
            "true"
        ],
        parameters=[
            {
                'use_sim_time': use_sim_time
            }
        ]
    )

    joint_state_broadcaster_spawner_node = Node(
        package = "controller_manager",
        executable = "spawner",
        arguments = ["joint_state_broadcaster", "-c", "/controller_manager"],
        parameters=[
            {
                'use_sim_time': use_sim_time
            }
        ]
    )

    forward_velocity_controller_spawner_node = Node(
        package = "controller_manager",
        executable = "spawner",
        arguments = ["forward_velocity_controller", "-c", "/controller_manager"],
        parameters=[
            {
                'use_sim_time': use_sim_time
            }
        ]
    )

    cmd_vel_parser_node = Node(
        package = "bot_control",
        executable = "cmd_vel_parser",
        parameters=[
            {
                'use_sim_time': use_sim_time
            }
        ]
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
            "world_file",
            default_value = PathJoinSubstitution([FindPackageShare("bot_control"), "worlds", "slam_test1.sdf"]),
            description = "Gazebo world file (absolute path or filename from the gz sim worlds collection) containing a custom world.",
        ),
        sim_time_arg,
        
        rsp_node,
        gz_node,
        ros_gz_bridge_node,
        ros_image_bridge_node,
        entity_spawner_node,
        joint_state_broadcaster_spawner_node,
        forward_velocity_controller_spawner_node,
        cmd_vel_parser_node
    ])