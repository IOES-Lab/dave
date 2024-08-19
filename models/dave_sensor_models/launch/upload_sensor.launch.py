from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, RegisterEventHandler, LogInfo
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch.conditions import IfCondition
from launch_ros.substitutions import FindPackageShare
from launch.event_handlers import OnProcessExit
from launch_ros.actions import Node


def generate_launch_description():
    sensor_name = LaunchConfiguration("sensor_name")
    use_sim = LaunchConfiguration("use_sim")
    x = LaunchConfiguration("x")
    y = LaunchConfiguration("y")
    z = LaunchConfiguration("z")
    roll = LaunchConfiguration("R")
    pitch = LaunchConfiguration("P")
    yaw = LaunchConfiguration("Y")

    args = [
        DeclareLaunchArgument(
            "sensor_name",
            default_value="",
            description="Name of the sensor to load",
        ),
        DeclareLaunchArgument(
            "use_sim",
            default_value="true",
            description="Flag to indicate whether to use simulation",
        ),
        DeclareLaunchArgument(
            "x",
            default_value="0",
            description="Initial x position",
        ),
        DeclareLaunchArgument(
            "y",
            default_value="0",
            description="Initial y position",
        ),
        DeclareLaunchArgument(
            "z",
            default_value="0.0",
            description="Initial z position",
        ),
        DeclareLaunchArgument(
            "R",
            default_value="0.0",
            description="Initial roll",
        ),
        DeclareLaunchArgument(
            "P",
            default_value="0.0",
            description="Initial pitch",
        ),
        DeclareLaunchArgument(
            "Y",
            default_value="0.0",
            description="Initial yaw",
        ),
    ]

    description_file = PathJoinSubstitution(
        [
            FindPackageShare("dave_sensor_models"),
            "description",
            sensor_name,
            "model.sdf",
        ]
    )

    gz_spawner = Node(
        package="ros_gz_sim",
        executable="create",
        arguments=[
            "-name",
            sensor_name,
            "-file",
            description_file,
            "-x",
            x,
            "-y",
            y,
            "-z",
            z,
            "-R",
            roll,
            "-P",
            pitch,
            "-Y",
            yaw,
        ],
        output="both",
        condition=IfCondition(use_sim),
        parameters=[{"use_sim_time": use_sim}],
    )

    nodes = [gz_spawner]

    event_handlers = [
        RegisterEventHandler(
            OnProcessExit(target_action=gz_spawner, on_exit=LogInfo(msg="Model Uploaded"))
        )
    ]

    return LaunchDescription(args + nodes + event_handlers)
