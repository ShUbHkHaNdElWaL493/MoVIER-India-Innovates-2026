from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare

def generate_launch_description():

    mode = LaunchConfiguration("mode")
    use_sim_time = LaunchConfiguration('use_sim_time')

    config = PathJoinSubstitution(
        [FindPackageShare('bot_slam'), 'config']
    )

    
    map_file_path = PathJoinSubstitution(
        [FindPackageShare('bot_slam'), 'map', 'map.pbstream']
    )
    
    load_state_filename_arg = DeclareLaunchArgument(
        'load_state_filename',
        default_value=map_file_path,
        description='Path to the .pbstream map file to load for localization'
    )
    load_state_filename = LaunchConfiguration('load_state_filename')


    sim_node = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(PathJoinSubstitution(
            [FindPackageShare("bot_control"), "launch", "spawn.launch.py"]
        )),
        launch_arguments = {
            "mode" : mode,
        }.items()
    )
    
    sim_time_arg = DeclareLaunchArgument(
        'use_sim_time', 
        default_value='true',
        description='Use simulation (Gazebo) clock if true'
    )
    
    cartographer_node = Node(
        package='cartographer_ros',
        executable='cartographer_node',
        output='screen',
        parameters=[
            {
                'use_sim_time': use_sim_time
            }
        ],
        arguments=[
            '-configuration_directory', config,
            '-configuration_basename', 'localisation.lua',
            '-load_state_filename', load_state_filename,
        ],
        remappings=[
            ('scan', '/top_lidar_scan'),
            ('imu', '/base_imu'),
            ('odom', '/odom')
        ]
    )

    cartographer_occupancy_grid_node = Node(
        package='cartographer_ros',
        executable='cartographer_occupancy_grid_node',
        output='screen',
        parameters=[
            {
                'use_sim_time': use_sim_time
            },
            {
                'resolution': 0.05
            }
        ],
        remappings=[
            ('map', '/map')
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
        load_state_filename_arg,
        sim_time_arg,
        # sim_node,
        cartographer_node,
        cartographer_occupancy_grid_node
    ])