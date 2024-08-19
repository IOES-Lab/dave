# Copyright 2019 Open Source Robotics Foundation, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import os

from ament_index_python.packages import get_package_share_directory

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.actions import IncludeLaunchDescription
from launch.conditions import IfCondition
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration

from launch_ros.actions import Node


def generate_launch_description():
    pkg_dave_demos = get_package_share_directory("dave_demos")
    multibeam_sonar_sim = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(pkg_dave_demos, "launch", "dave_sensor.launch.py")
        ),
        launch_arguments={
            "namespace": "blueview_p900",
            "world_name": "dave_multibeam_sonar",
            "paused": "false",
            "x": "4",
            "z": "0.5",
            "yaw": "3.14",
        }.items(),
    )

    # RViz
    pkg_dave_multibeam_sonar_demo = get_package_share_directory("dave_multibeam_sonar_demo")
    rviz = Node(
        package="rviz2",
        executable="rviz2",
        arguments=[
            "-d",
            os.path.join(pkg_dave_multibeam_sonar_demo, "rviz", "multibeam_sonar.rviz"),
        ],
        condition=IfCondition(LaunchConfiguration("rviz")),
    )

    # Bridge
    bridge = Node(
        package="ros_gz_bridge",
        executable="parameter_bridge",
        arguments=[
            "/sensor/camera@sensor_msgs/msg/Image@gz.msgs.Image",
            "/sensor/camera_info@sensor_msgs/msg/CameraInfo@gz.msgs.CameraInfo",
            "/sensor/depth_camera@sensor_msgs/msg/Image@gz.msgs.Image",
            "/sensor/multibeam_sonar/point_cloud@sensor_msgs/msg/PointCloud2@gz.msgs.PointCloudPacked",
        ],
        output="screen",
    )

    # ! TODO: Add a static transform publisher for the multibeam sonar
    # TF (Not sure about this....)
    tf_node = Node(
        package="tf2_ros",
        executable="static_transform_publisher",
        arguments=[
            "--frame-id",
            "world",
            "--child-frame-id",
            "blueview_p900/blueview_p900_base_link/multibeam_sonar",
        ],
    )

    return LaunchDescription(
        [
            multibeam_sonar_sim,
            DeclareLaunchArgument("rviz", default_value="true", description="Open RViz."),
            bridge,
            rviz,
            tf_node,
        ]
    )
