# Copyright 2025 Open Source Robotics Foundation, Inc.
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

from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():

    # Bridge
    bridge = Node(
        package="ros_gz_bridge",
        executable="parameter_bridge",
        arguments=[
            "/sensor/camera@sensor_msgs/msg/Image@gz.msgs.Image",  # for the multibeam world
            "/sensor/camera_info@sensor_msgs/msg/CameraInfo@gz.msgs.CameraInfo",  # for the multibeam world
            "/sensor/depth_camera@sensor_msgs/msg/Image@gz.msgs.Image",  # for the multibeam world
            "/sensor/multibeam_sonar/point_cloud@sensor_msgs/msg/PointCloud2@gz.msgs.PointCloudPacked",  # the point cloud bridge, must match the model.sdf
        ],
        output="screen",
    )

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

    return LaunchDescription([bridge, tf_node])
