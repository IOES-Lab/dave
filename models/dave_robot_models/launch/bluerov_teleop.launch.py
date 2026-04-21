from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, LogInfo
from launch.conditions import IfCondition
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():
    namespace = LaunchConfiguration("namespace")
    use_web_joystick = LaunchConfiguration("use_web_joystick")
    joystick_ws_host = LaunchConfiguration("joystick_ws_host")
    joystick_ws_port = LaunchConfiguration("joystick_ws_port")
    joystick_topic = LaunchConfiguration("joystick_topic")
    keyboard_topic = LaunchConfiguration("keyboard_topic")
    mavros_namespace = LaunchConfiguration("mavros_namespace")

    args = [
        DeclareLaunchArgument(
            "namespace",
            default_value="bluerov2",
            description="Gazebo model namespace/name",
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
            "joystick_topic",
            default_value="/joy",
            description="Joy topic fed to the manual control bridge",
        ),
        DeclareLaunchArgument(
            "keyboard_topic",
            default_value="/keyboard/joy",
            description="Joy topic emitted by keyboard teleop",
        ),
        DeclareLaunchArgument(
            "mavros_namespace",
            default_value="mavros",
            description="Namespace where MAVROS is running",
        ),
    ]

    keyboard_teleop = Node(
        package="dave_robot_models",
        executable="keyboard_publisher.py",
        output="screen",
        emulate_tty=True,
        parameters=[
            {
                "output_topic": keyboard_topic,
            }
        ],
    )

    websocket_teleop = Node(
        package="dave_robot_models",
        executable="ws_to_joy.py",
        output="screen",
        parameters=[
            {
                "ws_host": joystick_ws_host,
                "ws_port": joystick_ws_port,
                "output_topic": joystick_topic,
            }
        ],
        condition=IfCondition(use_web_joystick),
    )

    manual_control = Node(
        package="dave_robot_models",
        executable="ardusub_manual_control.py",
        output="screen",
        parameters=[
            {
                "model_name": namespace,
                "joystick_topic": joystick_topic,
                "keyboard_topic": keyboard_topic,
                "mavros_namespace": mavros_namespace,
            }
        ],
    )

    web_hint = LogInfo(
        msg=[
            "Web joystick enabled. Open extras/virtual_joystick.html and "
            "connect to ws://127.0.0.1:",
            joystick_ws_port,
        ],
        condition=IfCondition(use_web_joystick),
    )

    return LaunchDescription(
        args
        + [
            keyboard_teleop,
            websocket_teleop,
            manual_control,
            web_hint,
        ]
    )
