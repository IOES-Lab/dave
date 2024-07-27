import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import SetEnvironmentVariable
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch_ros.actions import Node
from launch.actions import ExecuteProcess

def generate_launch_description():
    package_share_directory = get_package_share_directory('dave_gz_model_plugins')
    model_path = os.path.join(package_share_directory, 'test')
    battery_world_path = os.path.join(model_path, 'battery_world.sdf')

    return LaunchDescription([

        SetEnvironmentVariable('GAZEBO_MODEL_PATH', model_path),

        ExecuteProcess(
            cmd=['gz', 'sim', battery_world_path],
            output='screen'),

        Node(
            package='dave_gz_model_plugins',
            executable='battery_state_listener.py',
            name='battery_state_listener',
            output='screen',
            parameters=[{'battery_state_topic': 'battery_state'}]
        )
    ])


# import os
# from ament_index_python.packages import get_package_share_directory
# from launch import LaunchDescription
# from launch.actions import DeclareLaunchArgument, LogInfo
# from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
# from launch_ros.actions import Node
# from launch.actions import ExecuteProcess

# def generate_launch_description():
#     # Get the path to the world file
#     package_share_directory = get_package_share_directory('dave_gz_model_plugins')
#     battery_world_path = os.path.join(package_share_directory, 'test', 'battery_world.sdf')

#     # Define the launch description
#     return LaunchDescription([
#         # Launch the Gazebo simulation
#         ExecuteProcess(
#             cmd=['gz', 'sim', battery_world_path],
#             output='screen',
#             name='gazebo_simulator'
#         ),

#         # Launch the ROS 2 node
#         Node(
#             package='dave_gz_model_plugins',
#             executable='linear_battery_plugin',  # Ensure this matches the executable in setup.py
#             name='linear_battery_plugin_node',
#             output='screen'
#         ),

#         # Optional: Log the paths and package details
#         LogInfo(
#             msg=f"Launching Gazebo simulation with world file: {battery_world_path}"
#         ),
#     ])

