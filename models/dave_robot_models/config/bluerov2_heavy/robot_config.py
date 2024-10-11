from launch import LaunchDescription
from launch.actions import (
    DeclareLaunchArgument,
    OpaqueFunction,
    ExecuteProcess,
)
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare


def launch_setup(context, *args, **kwargs):
    namespace = LaunchConfiguration("namespace").perform(context)

    thruster_joints = []
    for thruster in range(1, 9):
        thruster_joints.append(f"/model/{namespace}/joint/thruster{thruster}_joint")

    bluerov2_heavy_arguments = (
        [f"{joint}/cmd_thrust@std_msgs/msg/Float64@gz.msgs.Double" for joint in thruster_joints]
        + [f"{joint}/ang_vel@std_msgs/msg/Float64@gz.msgs.Double" for joint in thruster_joints]
        + [
            f"{joint}/enable_deadband@std_msgs/msg/Bool@gz.msgs.Boolean"
            for joint in thruster_joints
        ]
        + [
            f"/model/{namespace}/odometry@nav_msgs/msg/Odometry@gz.msgs.Odometry",
            f"/model/{namespace}/odometry_with_covariance@nav_msgs/msg/Odometry@gz.msgs.OdometryWithCovariance",
            f"/model/{namespace}/pose@geometry_msgs/msg/PoseArray@gz.msgs.Pose_V",
            f"/model/{namespace}/imu@sensor_msgs/msg/Imu@gz.msgs.IMU",
            f"/model/{namespace}/magnetometer@sensor_msgs/msg/MagneticField@gz.msgs.Magnetometer",
            f"/model/{namespace}/camera/image@sensor_msgs/msg/Image@gz.msgs.Image",
            f"/model/{namespace}/camera/camera_info@sensor_msgs/msg/CameraInfo@gz.msgs.CameraInfo",
        ]
    )

    bluerov2_heavy_bridge = Node(
        package="ros_gz_bridge",
        executable="parameter_bridge",
        arguments=bluerov2_heavy_arguments,
        output="screen",
    )

    mavros_file = LaunchConfiguration("mavros_file")

    mavros_node = Node(
        package="mavros",
        executable="mavros_node",
        output="screen",
        parameters=[mavros_file, {"use_sim_time": True}],
    )

    nodes = [bluerov2_heavy_bridge, mavros_node]

    ardusub_params = LaunchConfiguration("ardusub_params").perform(context)

    ardusub_cmd = [
        "ardusub -S -w -M gazebo --defaults "
        + ardusub_params
        + " -IO --home 44.65870,-124.06556,0.0,270.0"
    ]

    ardusub_process = ExecuteProcess(cmd=ardusub_cmd, shell=True, output="screen")

    processes = [ardusub_process]

    return nodes + processes


def generate_launch_description():
    args = [
        DeclareLaunchArgument(
            "namespace",
            default_value="",
            description="Namespace",
        ),
        DeclareLaunchArgument(
            "mavros_file",
            default_value=PathJoinSubstitution(
                [FindPackageShare("dave_robot_models"), "config", "mavros", "mavros.yaml"]
            ),
            description="Path to mavros.yaml file",
        ),
        DeclareLaunchArgument(
            "ardusub_params",
            default_value=PathJoinSubstitution(
                [FindPackageShare("dave_robot_models"), "config", "bluerov2", "ardusub.parm"]
            ),
            description="Path to ardusub.parm file",
        ),
    ]

    return LaunchDescription(args + [OpaqueFunction(function=launch_setup)])
