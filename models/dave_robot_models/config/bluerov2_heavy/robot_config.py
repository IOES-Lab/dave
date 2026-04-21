from launch import LaunchDescription
from launch.actions import (
    DeclareLaunchArgument,
    ExecuteProcess,
    IncludeLaunchDescription,
    LogInfo,
    OpaqueFunction,
    RegisterEventHandler,
)
from launch.conditions import IfCondition
from launch.event_handlers import OnProcessExit, OnProcessStart
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare


def launch_setup(context, *args, **kwargs):
    namespace = LaunchConfiguration("namespace").perform(context)
    use_ardusub = LaunchConfiguration("use_ardusub")
    use_teleop = LaunchConfiguration("use_teleop")
    use_web_joystick = LaunchConfiguration("use_web_joystick")
    joystick_ws_host = LaunchConfiguration("joystick_ws_host")
    joystick_ws_port = LaunchConfiguration("joystick_ws_port")
    zoom_camera = LaunchConfiguration("zoom_camera")
    zoom_camera_delay = LaunchConfiguration("zoom_camera_delay").perform(context)
    open_qgc = LaunchConfiguration("open_qgc")
    open_virtual_joystick = LaunchConfiguration("open_virtual_joystick")
    virtual_joystick_url = LaunchConfiguration("virtual_joystick_url").perform(context)
    ui_launch_delay = LaunchConfiguration("ui_launch_delay").perform(context)

    # ArduPilotPlugin already publishes the internal actuator topics on Gazebo transport.
    # Bridging them here creates a second GZ publisher with no ROS producer behind it.
    bluerov2_heavy_arguments = [
        "/clock@rosgraph_msgs/msg/Clock[gz.msgs.Clock",
        f"/model/{namespace}/odometry@nav_msgs/msg/Odometry[gz.msgs.Odometry",
        (
            f"/model/{namespace}/odometry_with_covariance@"
            "nav_msgs/msg/Odometry[gz.msgs.OdometryWithCovariance"
        ),
        f"/model/{namespace}/pose@geometry_msgs/msg/PoseArray[gz.msgs.Pose_V",
        f"/model/{namespace}/imu@sensor_msgs/msg/Imu[gz.msgs.IMU",
        f"/model/{namespace}/magnetometer@sensor_msgs/msg/MagneticField[gz.msgs.Magnetometer",
    ]

    bluerov2_heavy_bridge = Node(
        package="ros_gz_bridge",
        executable="parameter_bridge",
        arguments=bluerov2_heavy_arguments,
        output="screen",
    )

    mavros_file = LaunchConfiguration("mavros_file")
    ardusub_params = LaunchConfiguration("ardusub_params")
    ardusub_model = LaunchConfiguration("ardusub_model")
    ardusub_home = LaunchConfiguration("ardusub_home")

    imu_wait_cmd = (
        "while true; do "
        f'if gz topic -l | grep -q "/model/{namespace}/imu"; then exit 0; fi; '
        "sleep 1; "
        "done"
    )

    wait_for_imu_topic = ExecuteProcess(
        cmd=[
            "/usr/bin/env",
            "bash",
            "-lc",
            imu_wait_cmd,
        ],
        output="screen",
        condition=IfCondition(use_ardusub),
    )

    zoom_camera_cmd = (
        f"sleep {zoom_camera_delay}; "
        "gz service -s /gui/move_to/pose --reqtype gz.msgs.GUICamera "
        "--reptype gz.msgs.Boolean --timeout 500 "
        "--req 'pose: {position:{x:-1, y:0, z:0.5}, "
        "orientation:{x:0.0, y:0.0, z:0.0, w:0.0}}'"
    )

    zoom_camera_process = ExecuteProcess(
        cmd=[
            "/usr/bin/env",
            "bash",
            "-lc",
            zoom_camera_cmd,
        ],
        output="screen",
        condition=IfCondition(zoom_camera),
    )

    qgc_cmd = (
        "if ! command -v qgroundcontrol >/dev/null 2>&1; then "
        'echo "QGroundControl not found, skipping open_qgc."; '
        "exit 0; "
        "fi; "
        f"sleep {ui_launch_delay}; "
        "while true; do "
        'if [ "$(id -u)" -eq 0 ]; then '
        "sudo -u ubuntu qgroundcontrol; "
        "else "
        "qgroundcontrol; "
        "fi; "
        "sleep 3; "
        "done"
    )
    qgc_process = ExecuteProcess(
        cmd=[
            "/usr/bin/env",
            "bash",
            "-lc",
            qgc_cmd,
        ],
        output="screen",
        condition=IfCondition(open_qgc),
    )

    joystick_cmd = (
        "if ! command -v firefox >/dev/null 2>&1; then "
        'echo "Firefox not found, skipping open_virtual_joystick."; '
        "exit 0; "
        "fi; "
        f"sleep {ui_launch_delay}; "
        'if [ "$(id -u)" -eq 0 ]; then '
        f"sudo -u ubuntu firefox --new-window '{virtual_joystick_url}'; "
        "else "
        f"firefox --new-window '{virtual_joystick_url}'; "
        "fi"
    )
    joystick_process = ExecuteProcess(
        cmd=[
            "/usr/bin/env",
            "bash",
            "-lc",
            joystick_cmd,
        ],
        output="screen",
        condition=IfCondition(open_virtual_joystick),
    )

    ardusub_process = ExecuteProcess(
        cmd=[
            "ardusub",
            "-w",
            "--model",
            ardusub_model,
            "--defaults",
            ardusub_params,
            "-IO",
            "--home",
            ardusub_home,
        ],
        output="screen",
        condition=IfCondition(use_ardusub),
    )

    mavros_node = Node(
        package="mavros",
        executable="mavros_node",
        output="screen",
        parameters=[mavros_file, {"use_sim_time": True}],
        condition=IfCondition(use_ardusub),
    )

    start_imu_wait = RegisterEventHandler(
        OnProcessStart(
            target_action=bluerov2_heavy_bridge,
            on_start=[wait_for_imu_topic],
        )
    )

    start_ardusub = RegisterEventHandler(
        OnProcessExit(
            target_action=wait_for_imu_topic,
            on_exit=[
                LogInfo(msg="Gazebo IMU topic detected, launching ArduSub"),
                ardusub_process,
            ],
        )
    )

    start_mavros = RegisterEventHandler(
        OnProcessStart(
            target_action=ardusub_process,
            on_start=[
                LogInfo(msg="ArduSub started, launching MAVROS"),
                mavros_node,
            ],
        )
    )

    teleop_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            [
                PathJoinSubstitution(
                    [
                        FindPackageShare("dave_robot_models"),
                        "launch",
                        "bluerov_teleop.launch.py",
                    ]
                )
            ]
        ),
        launch_arguments={
            "namespace": LaunchConfiguration("namespace"),
            "use_web_joystick": use_web_joystick,
            "joystick_ws_host": joystick_ws_host,
            "joystick_ws_port": joystick_ws_port,
            "joystick_topic": "/joy",
            "keyboard_topic": "/keyboard/joy",
            "mavros_namespace": "mavros",
        }.items(),
        condition=IfCondition(use_teleop),
    )

    return [
        bluerov2_heavy_bridge,
        start_imu_wait,
        start_ardusub,
        start_mavros,
        teleop_launch,
        zoom_camera_process,
        qgc_process,
        joystick_process,
    ]


def generate_launch_description():
    args = [
        DeclareLaunchArgument(
            "namespace",
            default_value="bluerov2_heavy",
            description="Namespace",
        ),
        DeclareLaunchArgument(
            "mavros_file",
            default_value=PathJoinSubstitution(
                [
                    FindPackageShare("dave_robot_models"),
                    "config",
                    "mavros",
                    "mavros.yaml",
                ]
            ),
            description="Path to mavros.yaml file",
        ),
        DeclareLaunchArgument(
            "use_ardusub",
            default_value="true",
            description="Launch ArduSub SITL and MAVROS",
        ),
        DeclareLaunchArgument(
            "use_teleop",
            default_value="true",
            description="Launch keyboard and websocket teleop nodes",
        ),
        DeclareLaunchArgument(
            "use_web_joystick",
            default_value="true",
            description="Launch websocket joystick bridge",
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
            "ardusub_params",
            default_value=PathJoinSubstitution(
                [
                    FindPackageShare("dave_robot_models"),
                    "config",
                    "bluerov2",
                    "ardusub.parm",
                ]
            ),
            description="Path to ardusub.parm file",
        ),
        DeclareLaunchArgument(
            "ardusub_model",
            default_value="JSON:127.0.0.1",
            description="ArduSub SITL physics backend model string",
        ),
        DeclareLaunchArgument(
            "ardusub_home",
            default_value="35.074823,129.084798,0.0,270.0",
            description="ArduSub HOME argument (lat,lon,alt,heading)",
        ),
        DeclareLaunchArgument(
            "zoom_camera",
            default_value="false",
            description="Zoom the GUI camera after launch",
        ),
        DeclareLaunchArgument(
            "zoom_camera_delay",
            default_value="2.0",
            description="Delay (seconds) before moving the GUI camera",
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
            default_value=(
                "https://raw.githubusercontent.com/IOES-Lab/dave/"
                "refs/heads/ros2/extras/virtual_joystick.html"
            ),
            description="URL for the virtual joystick page",
        ),
        DeclareLaunchArgument(
            "ui_launch_delay",
            default_value="2.0",
            description="Delay (seconds) before launching QGC/Firefox",
        ),
    ]

    return LaunchDescription(args + [OpaqueFunction(function=launch_setup)])
