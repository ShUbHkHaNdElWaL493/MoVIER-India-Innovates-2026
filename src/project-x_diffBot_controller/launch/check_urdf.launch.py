# import os
# from ament_index_python.packages import get_package_share_directory
# from launch import LaunchDescription
# from launch.actions import DeclareLaunchArgument
# from launch.substitutions import LaunchConfiguration
# from launch_ros.actions import Node

# def generate_launch_description():
#     # Define paths
#     # Note: Using absolute path for URDF since it's not in a package yet
#     urdf_file = '/home/projx/controller/src/project-x_diffBot_controller/description/robot.urdf'
    
#     with open(urdf_file, 'r') as infp:
#         robot_desc = infp.read()

#     # Create the launch description
#     return LaunchDescription([
#         Node(
#             package='robot_state_publisher',
#             executable='robot_state_publisher',
#             name='robot_state_publisher',
#             output='screen',
#             parameters=[{'robot_description': robot_desc}]
#         ),
#         # Node(
#         #     package='joint_state_publisher_gui',
#         #     executable='joint_state_publisher_gui',
#         #     name='joint_state_publisher_gui'
#         # ),
#         Node(
#             package='rviz2',
#             executable='rviz2',
#             name='rviz2',
#             output='screen'
#         )
#     ])

#     # ros2 launch /home/hitech/bot_dp_2/display.launch.py


import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node

def generate_launch_description():
    # Define paths
    urdf_file = '/home/orinmars/ayush_ws/src/src/project-x_diffBot_controller/description/robot.urdf'
    with open(urdf_file, 'r') as infp:
        robot_desc = infp.read()

    # Declare the launch argument so you can change it from the terminal
    use_sim_time = LaunchConfiguration('use_sim_time')
    declare_use_sim_time_cmd = DeclareLaunchArgument(
        'use_sim_time',
        default_value='true',
        description='Use simulation (Gazebo) clock if true')

    # Create the launch description
    return LaunchDescription([
        declare_use_sim_time_cmd,

        Node(
            package='robot_state_publisher',
            executable='robot_state_publisher',
            name='robot_state_publisher',
            output='screen',
            parameters=[{
                'robot_description': robot_desc,
                'use_sim_time': use_sim_time,
                'publish_frequency': 50.0  # Force higher update rate
            }]
        ),

        Node(
            package='rviz2',
            executable='rviz2',
            name='rviz2',
            output='screen',
            parameters=[{'use_sim_time': use_sim_time}]
        )
    ])
