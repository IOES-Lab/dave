from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, OpaqueFunction
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def launch_setup(context, *args, **kwargs):
    # Get the namespace from the launch arguments
    namespace = LaunchConfiguration("namespace").perform(context)

    # Build up the list of topics to bridge: thrust commands, angular velocity, deadband enable, etc.
    # Adjust these topics (add/remove) if you have additional sensors or different naming.
    wamv_arguments = (
        [
            f"/model/{namespace}/joint/left_engine_joint/cmd_force@std_msgs/msg/Float64@gz.msgs.Double",
            f"/model/{namespace}/joint/left_propeller_joint/cmd_thrust@std_msgs/msg/Float64@gz.msgs.Double",
            f"/model/{namespace}/joint/left_propeller_joint/ang_vel@std_msgs/msg/Float64@gz.msgs.Double",
            f"/model/{namespace}/joint/right_engine_joint/cmd_force@std_msgs/msg/Float64@gz.msgs.Double",
            f"/model/{namespace}/joint/right_propeller_joint/cmd_thrust@std_msgs/msg/Float64@gz.msgs.Double",
            f"/model/{namespace}/joint/right_propeller_joint/ang_vel@std_msgs/msg/Float64@gz.msgs.Double",
        ]
        + [
            f"/world/waves/model/{namespace}/link/imu_link/sensor/imu_sensor/imu@sensor_msgs/msg/Imu@gz.msgs.IMU",
            f"/model/{namespace}/pose@geometry_msgs/msg/PoseStamped[gz.msgs.Pose",
        ]
        + [
            "/world/waves/clock@rosgraph_msgs/msg/Clock@gz.msgs.Clock",
            "/clock@rosgraph_msgs/msg/Clock@gz.msgs.Clock",
        ]
        # + [
        #     f"/model/{namespace}/pose@geometry_msgs/msg/PoseArray@gz.msgs.Pose_V",
        #     # f"/model/{namespace}/camera/image@sensor_msgs/msg/Image@gz.msgs.Image",
        #     # f"/model/{namespace}/camera/camera_info@sensor_msgs/msg/CameraInfo@gz.msgs.CameraInfo",
        # ]
    )

    # Node that bridges the ROS-Gazebo topics
    wamv_bridge = Node(
        package="ros_gz_bridge",
        executable="parameter_bridge",
        arguments=wamv_arguments,
        output="screen",
    )

    nodes = [wamv_bridge]

    # Set up MAVROS if you’re using it for control
    # mavros_file = LaunchConfiguration("mavros_file")
    # mavros_node = Node(
    #     package="mavros",
    #     executable="mavros_node",
    #     output="screen",
    #     parameters=[mavros_file, {"use_sim_time": True}],
    # )

    # # If you’re using Ardusub (or an ArduPilot variant) for your surface vehicle,
    # # here’s an example call — adapt the command or parameters as needed.
    # ardusub_params = LaunchConfiguration("ardusub_params").perform(context)
    # ardusub_cmd = [
    #     "ardusub -S -w -M gazebo --defaults "
    #     + ardusub_params
    #     + " -IO --home 44.65870,-124.06556,0.0,270.0"
    # ]
    # ardusub_process = ExecuteProcess(cmd=ardusub_cmd, shell=True, output="screen")

    # processes = [ardusub_process]

    return nodes


def generate_launch_description():
    # Declare launch arguments for namespacing and parameter file paths
    args = [
        DeclareLaunchArgument(
            "namespace",
            default_value="wamv",
            description="Namespace to remap the model topics under",
        ),
        # DeclareLaunchArgument(
        #     "mavros_file",
        #     default_value=PathJoinSubstitution(
        #         [FindPackageShare("waves_models"), "models", namespace, "config", "mavros.yaml"]
        #     ),
        #     description="Path to mavros.yaml file",
        # ),
        # DeclareLaunchArgument(
        #     "ardusub_params",
        #     default_value=PathJoinSubstitution(
        #         [FindPackageShare("waves_models"), "models", namespace, "config", "ardupilot.parm"]
        #     ),
        #     description="Path to ardusub.parm file",
        # ),
    ]

    # We return a LaunchDescription that includes our nodes and processes
    return LaunchDescription(args + [OpaqueFunction(function=launch_setup)])
