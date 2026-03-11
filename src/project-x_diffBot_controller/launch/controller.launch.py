import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import RegisterEventHandler, DeclareLaunchArgument
from launch.event_handlers import OnProcessExit
from launch.substitutions import Command, LaunchConfiguration
from launch_ros.actions import Node

def generate_launch_description():
    pkg_share = get_package_share_directory('project-x_diffbot_controller')

    # --- PATHS ---
    # 1. Your raw URDF (Visuals)
    urdf_file_path = os.path.join(pkg_share, 'description', 'robot.urdf')
    
    # 2. Your Hardware Interface (Control)
    hardware_file_path = os.path.join(pkg_share, 'description', 'ros_control_hardware.xacro')
    
    # 3. Config
    controller_config = os.path.join(pkg_share, 'config', 'controller_config.yaml')

    # --- MANUAL MERGE LOGIC ---
    # Read the URDF
    with open(urdf_file_path, 'r') as file:
        urdf_content = file.read()

    # Read the Hardware Xacro
    # We need to strip the <robot> tags from this file so we just get the <ros2_control> block
    with open(hardware_file_path, 'r') as file:
        hardware_content = file.read()
        
    # Remove <robot> wrapper from hardware file if present to avoid nested tags
    # This is a simple string hack: find the start and end of <ros2_control>
    start_tag = "<ros2_control"
    end_tag = "</ros2_control>"
    
    start_idx = hardware_content.find(start_tag)
    end_idx = hardware_content.find(end_tag) + len(end_tag)
    
    if start_idx != -1 and end_idx != -1:
        # Extract just the ros2_control block
        clean_hardware_content = hardware_content[start_idx:end_idx]
    else:
        # Fallback: assume the file is ONLY the tag
        clean_hardware_content = hardware_content

    # INJECT HARDWARE INTO URDF
    # We insert the control block right before the closing </robot> tag of the main URDF
    final_robot_description = urdf_content.replace('</robot>', clean_hardware_content + '\n</robot>')

    # Create the parameter object
    robot_desc_param = {'robot_description': final_robot_description}

    # --- NODES ---

    # 1. Robot State Publisher
    robot_state_publisher_node = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        output='both',
        parameters=[robot_desc_param]
    )

    # 2. Controller Manager
    control_node = Node(
        package="controller_manager",
        executable="ros2_control_node",
        parameters=[robot_desc_param, controller_config],
        output="both",
        remappings=[
            ("~/robot_description", "/robot_description"),
            ("/diffbot_base_controller/cmd_vel_unstamped", "/cmd_vel"),
        ],
    )

    # 3. Spawners
    joint_state_broadcaster_spawner = Node(
        package="controller_manager",
        executable="spawner",
        arguments=["joint_state_broadcaster", "--controller-manager", "/controller_manager"],
    )

    robot_controller_spawner = Node(
        package="controller_manager",
        executable="spawner",
        arguments=["diffbot_base_controller", "--controller-manager", "/controller_manager"],
    )

    # 4. RViz
    rviz_node = Node(
        package="rviz2",
        executable="rviz2",
        name="rviz2",
        output="log",
        arguments=["-d", os.path.join(pkg_share, 'description', 'diffbot_view.rviz')],
        parameters=[robot_desc_param]
    )

    # --- ORCHESTRATION ---
    delay_robot_controller = RegisterEventHandler(
        event_handler=OnProcessExit(
            target_action=joint_state_broadcaster_spawner,
            on_exit=[robot_controller_spawner],
        )
    )

    return LaunchDescription([
        robot_state_publisher_node,
        control_node,
        joint_state_broadcaster_spawner,
        delay_robot_controller,
        rviz_node
    ])