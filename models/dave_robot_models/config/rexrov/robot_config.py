from launch import LaunchDescription
from launch_ros.actions import Node
from launch.substitutions import LaunchConfiguration
from launch.actions import DeclareLaunchArgument, OpaqueFunction


def launch_setup(context, *args, **kwargs):
    namespace = LaunchConfiguration("namespace").perform(context)

    thruster_joints = []
    for thruster in range(1, 9):
        thruster_joints.append(f"/model/{namespace}/joint/thruster{thruster}_joint")

    rexrov_arguments = (
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

    rexrov_bridge = Node(
        package="ros_gz_bridge",
        executable="parameter_bridge",
        arguments=rexrov_arguments,
        output="screen",
    )

    return [rexrov_bridge]


def generate_launch_description():
    args = [
        DeclareLaunchArgument(
            "namespace",
            default_value="",
            description="Namespace",
        ),
    ]

    return LaunchDescription(args + [OpaqueFunction(function=launch_setup)])
