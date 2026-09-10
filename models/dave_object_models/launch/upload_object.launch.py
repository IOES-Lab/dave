import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, RegisterEventHandler, LogInfo, OpaqueFunction
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch.conditions import IfCondition
from launch_ros.substitutions import FindPackageShare
from launch.event_handlers import OnProcessExit
from launch_ros.actions import Node


def validate_description(context, *args, **kwargs):
    namespace = LaunchConfiguration("namespace").perform(context)
    description_file = os.path.join(
        get_package_share_directory("dave_object_models"),
        "description",
        namespace,
        "model.sdf",
    )
    if not os.path.isfile(description_file):
        raise FileNotFoundError(
            f"Object descriptor does not exist for namespace [{namespace}]: " f"{description_file}"
        )
    return []


def generate_launch_description():
    use_sim_time = LaunchConfiguration("use_sim_time")
    namespace = LaunchConfiguration("namespace")
    x = LaunchConfiguration("x")
    y = LaunchConfiguration("y")
    z = LaunchConfiguration("z")
    roll = LaunchConfiguration("roll")
    pitch = LaunchConfiguration("pitch")
    yaw = LaunchConfiguration("yaw")
    use_ned_frame = LaunchConfiguration("use_ned_frame")

    args = [
        DeclareLaunchArgument(
            "gui",
            default_value="true",
            description="Flag to indicate whether to use simulation",
        ),
        DeclareLaunchArgument(
            "use_sim_time",
            default_value="true",
            description="Flag to indicate whether to use sim time",
        ),
        DeclareLaunchArgument(
            "namespace",
            default_value="",
            description="Namespace",
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
            "roll",
            default_value="0.0",
            description="Initial roll",
        ),
        DeclareLaunchArgument(
            "pitch",
            default_value="0.0",
            description="Initial pitch",
        ),
        DeclareLaunchArgument(
            "yaw",
            default_value="0.0",
            description="Initial yaw",
        ),
        DeclareLaunchArgument(
            "use_ned_frame",
            default_value="false",
            description="Use North-East-Down frame",
        ),
    ]

    description_file = PathJoinSubstitution(
        [
            FindPackageShare("dave_object_models"),
            "description",
            namespace,
            "model.sdf",
        ]
    )

    tf2_spawner = Node(
        package="tf2_ros",
        executable="static_transform_publisher",
        name="world_to_world_ned",
        arguments=[
            "--roll",
            "1.57",
            "--yaw",
            "3.14",
            "--frame_id",
            "world",
            "--child_frame_id",
            "world_ned",
        ],
        output="both",
        condition=IfCondition(use_ned_frame),
        parameters=[{"use_sim_time": use_sim_time}],
    )

    gz_spawner = Node(
        package="ros_gz_sim",
        executable="create",
        arguments=[
            "-name",
            namespace,
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
        parameters=[{"use_sim_time": use_sim_time}],
    )

    nodes = [tf2_spawner, gz_spawner]

    event_handlers = [
        RegisterEventHandler(
            OnProcessExit(target_action=gz_spawner, on_exit=LogInfo(msg="Object Model Uploaded"))
        )
    ]

    validate_object_description = OpaqueFunction(function=validate_description)

    return LaunchDescription(args + [validate_object_description] + nodes + event_handlers)
