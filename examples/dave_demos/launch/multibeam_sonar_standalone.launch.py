import os
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.conditions import IfCondition
from launch_ros.substitutions import FindPackageShare
from ament_index_python.packages import get_package_share_directory


# ros2 launch dave_demos multibeam_sonar_standalone.launch.py x:=4.0 z:=0.5 Y:=3.14
def generate_launch_description():
    use_sim = LaunchConfiguration("use_sim")
    sensor_name = LaunchConfiguration("sensor_name")

    pkg_dave_worlds = get_package_share_directory("dave_worlds")
    world_path = os.path.join(pkg_dave_worlds, "worlds", "dave_multibeam_sonar.world")

    # Declare the launch arguments with default values
    args = [
        DeclareLaunchArgument(
            "world_name",
            default_value="dave_multibeam_sonar",
            description="Gazebo world file to launch",
        ),
        DeclareLaunchArgument(
            "use_sim",
            default_value="true",
            description="Flag to indicate whether to use simulation",
        ),
        DeclareLaunchArgument(
            "sensor_name",
            default_value="nortek_dvl500_300_with_multibeam_sonar",
            description="Name of the model to load",
        ),
    ]

    # Include the first launch file
    gz_sim_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            [
                PathJoinSubstitution(
                    [
                        FindPackageShare("ros_gz_sim"),
                        "launch",
                        "gz_sim.launch.py",
                    ]
                )
            ]
        ),
        launch_arguments=[
            (
                "gz_args",
                ["-r ", world_path],
            ),
        ],
        condition=IfCondition(use_sim),
    )

    # Include the second launch file with model name
    model_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            [
                PathJoinSubstitution(
                    [
                        FindPackageShare("dave_sensor_models"),
                        "launch",
                        "upload_sensor.launch.py",
                    ]
                )
            ]
        ),
        launch_arguments={
            "sensor_name": sensor_name,
            "use_sim": use_sim,
        }.items(),
    )

    include = [gz_sim_launch, model_launch]

    return LaunchDescription(args + include)
