from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription, OpaqueFunction
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution, TextSubstitution
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.conditions import IfCondition
from launch_ros.substitutions import FindPackageShare


def launch_setup(context, *args, **kwargs):
    paused = LaunchConfiguration("paused").perform(context)
    gui = LaunchConfiguration("gui").perform(context)
    use_sim_time = LaunchConfiguration("use_sim_time").perform(context)
    headless = LaunchConfiguration("headless").perform(context)
    verbose = LaunchConfiguration("verbose").perform(context)
    namespace = LaunchConfiguration("namespace").perform(context)
    world_name = LaunchConfiguration("world_name").perform(context)
    x = LaunchConfiguration("x").perform(context)
    y = LaunchConfiguration("y").perform(context)
    z = LaunchConfiguration("z").perform(context)
    roll = LaunchConfiguration("roll").perform(context)
    pitch = LaunchConfiguration("pitch").perform(context)
    yaw = LaunchConfiguration("yaw").perform(context)
    use_ned_frame = LaunchConfiguration("use_ned_frame").perform(context)

    if world_name != "empty.sdf":
        world_filename = f"{world_name}.world"
        world_filepath = PathJoinSubstitution(
            [FindPackageShare("dave_worlds"), "worlds", world_filename]
        ).perform(context)
        gz_args = [world_filepath]
    else:
        gz_args = [world_name]

    if headless == "true":
        gz_args.append("-s")
    if paused == "false":
        gz_args.append("-r")
    if verbose == "true":
        gz_args.append("--verbose")

    gz_args_str = " ".join(gz_args)

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
            ("gz_args", TextSubstitution(text=gz_args_str)),
        ],
        condition=IfCondition(gui),
    )

    sensor_launch = IncludeLaunchDescription(
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
            "gui": gui,
            "use_sim_time": use_sim_time,
            "namespace": namespace,
            "x": x,
            "y": y,
            "z": z,
            "roll": roll,
            "pitch": pitch,
            "yaw": yaw,
            "use_ned_frame": use_ned_frame,
        }.items(),
    )

    return [gz_sim_launch, sensor_launch]


def generate_launch_description():

    args = [
        DeclareLaunchArgument(
            "paused",
            default_value="true",
            description="Start the simulation paused",
        ),
        DeclareLaunchArgument(
            "gui",
            default_value="true",
            description="Flag to enable the gazebo gui",
        ),
        DeclareLaunchArgument(
            "use_sim_time",
            default_value="true",
            description="Flag to indicate whether to use simulation time",
        ),
        DeclareLaunchArgument(
            "headless",
            default_value="false",
            description="Flag to enable the gazebo headless mode",
        ),
        DeclareLaunchArgument(
            "verbose",
            default_value="false",
            description="Enable verbose mode for Gazebo simulation",
        ),
        DeclareLaunchArgument(
            "world_name",
            default_value="empty.sdf",
            description="Gazebo world file to launch",
        ),
        DeclareLaunchArgument(
            "namespace",
            default_value="",
            description="Namespace",
        ),
        DeclareLaunchArgument(
            "x",
            default_value="0.0",
            description="Initial x position",
        ),
        DeclareLaunchArgument(
            "y",
            default_value="0.0",
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
            description="Initial roll angle",
        ),
        DeclareLaunchArgument(
            "pitch",
            default_value="0.0",
            description="Initial pitch angle",
        ),
        DeclareLaunchArgument(
            "yaw",
            default_value="0.0",
            description="Initial yaw angle",
        ),
        DeclareLaunchArgument(
            "use_ned_frame",
            default_value="false",
            description="Flag to indicate whether to use the north-east-down frame",
        ),
    ]

    return LaunchDescription(args + [OpaqueFunction(function=launch_setup)])
