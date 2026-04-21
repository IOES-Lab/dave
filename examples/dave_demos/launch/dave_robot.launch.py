from launch import LaunchDescription
from launch.actions import (
    DeclareLaunchArgument,
    IncludeLaunchDescription,
    OpaqueFunction,
)
from launch.conditions import IfCondition
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch_ros.substitutions import FindPackageShare


def launch_setup(context, *args, **kwargs):
    paused = LaunchConfiguration("paused")
    gui = LaunchConfiguration("gui")
    use_sim_time = LaunchConfiguration("use_sim_time")
    debug = LaunchConfiguration("debug")
    headless = LaunchConfiguration("headless")
    verbose = LaunchConfiguration("verbose")
    namespace = LaunchConfiguration("namespace")
    world_name = LaunchConfiguration("world_name")
    zoom_camera_delay = LaunchConfiguration("zoom_camera_delay")
    x = LaunchConfiguration("x")
    y = LaunchConfiguration("y")
    z = LaunchConfiguration("z")
    roll = LaunchConfiguration("roll")
    pitch = LaunchConfiguration("pitch")
    yaw = LaunchConfiguration("yaw")
    use_ned_frame = LaunchConfiguration("use_ned_frame")
    use_teleop = LaunchConfiguration("use_teleop")
    use_web_joystick = LaunchConfiguration("use_web_joystick")
    joystick_ws_host = LaunchConfiguration("joystick_ws_host")
    joystick_ws_port = LaunchConfiguration("joystick_ws_port")
    open_qgc = LaunchConfiguration("open_qgc")
    open_virtual_joystick = LaunchConfiguration("open_virtual_joystick")
    virtual_joystick_url = LaunchConfiguration("virtual_joystick_url")
    ui_launch_delay = LaunchConfiguration("ui_launch_delay")

    selected_world_name = LaunchConfiguration("world_name").perform(context)
    if selected_world_name != "empty.sdf":
        world_filename = f"{selected_world_name}.world"
        world_filepath = PathJoinSubstitution(
            [FindPackageShare("dave_worlds"), "worlds", world_filename]
        )
        gz_args = [world_filepath]
    else:
        gz_args = [world_name]

    zoom_camera_value = "true" if selected_world_name == "dave_ocean_waves" else "false"

    if headless.perform(context) == "true":
        gz_args.append(" -s")
    if paused.perform(context) == "false":
        gz_args.append(" -r")
    if debug.perform(context) == "true":
        gz_args.append(" -v ")
        gz_args.append(verbose.perform(context))

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
            ("gz_args", gz_args),
        ],
        condition=IfCondition(gui),
    )

    # Include the second launch file with model name
    robot_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            [
                PathJoinSubstitution(
                    [
                        FindPackageShare("dave_robot_models"),
                        "launch",
                        "upload_robot.launch.py",
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
            "use_teleop": use_teleop,
            "use_web_joystick": use_web_joystick,
            "joystick_ws_host": joystick_ws_host,
            "joystick_ws_port": joystick_ws_port,
            "zoom_camera": zoom_camera_value,
            "zoom_camera_delay": zoom_camera_delay,
            "open_qgc": open_qgc,
            "open_virtual_joystick": open_virtual_joystick,
            "virtual_joystick_url": virtual_joystick_url,
            "ui_launch_delay": ui_launch_delay,
        }.items(),
    )

    include = [gz_sim_launch, robot_launch]

    return include


def generate_launch_description():

    # Declare the launch arguments with default values
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
            "debug",
            default_value="false",
            description="Flag to enable the gazebo debug flag",
        ),
        DeclareLaunchArgument(
            "headless",
            default_value="false",
            description="Flag to enable the gazebo headless mode",
        ),
        DeclareLaunchArgument(
            "verbose",
            default_value="0",
            description="Adjust level of console verbosity",
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
        DeclareLaunchArgument(
            "use_teleop",
            default_value="true",
            description="Launch BlueROV teleop bridge and keyboard controls",
        ),
        DeclareLaunchArgument(
            "use_web_joystick",
            default_value="true",
            description="Launch websocket joystick bridge for virtual joystick",
        ),
        DeclareLaunchArgument(
            "joystick_ws_host",
            default_value="0.0.0.0",
            description="Bind host for websocket joystick bridge",
        ),
        DeclareLaunchArgument(
            "joystick_ws_port",
            default_value="8765",
            description="Bind port for websocket joystick bridge",
        ),
        DeclareLaunchArgument(
            "open_qgc",
            default_value="false",
            description="Launch QGroundControl",
        ),
        DeclareLaunchArgument(
            "open_virtual_joystick",
            default_value="false",
            description="Open the virtual joystick page in Firefox",
        ),
        DeclareLaunchArgument(
            "virtual_joystick_url",
            default_value=("https://ioes-lab.github.io/dave/extras/virtual_joystick.html"),
            description="URL for the virtual joystick page",
        ),
        DeclareLaunchArgument(
            "ui_launch_delay",
            default_value="2.0",
            description="Delay (seconds) before launching QGC/Firefox",
        ),
        DeclareLaunchArgument(
            "zoom_camera_delay",
            default_value="2.0",
            description="Delay (seconds) before moving the GUI camera",
        ),
    ]

    return LaunchDescription(args + [OpaqueFunction(function=launch_setup)])
