# External BlueROV2 Direct-Thruster Path Controller

This note points to a community-maintained ROS 2 BlueROV2 path-following controller package
for DAVE / Gazebo:

[BlueROV2 reconciled path control](https://github.com/drwa92/bluerov2_reconciled_path_control_ros2)

> [!NOTE]
> This package is developed and released outside DAVE. Startup and package-discovery
> fixes found during DAVE validation are tracked in the
> [external compatibility PR](https://github.com/drwa92/bluerov2_reconciled_path_control_ros2/pull/1).
> Check that PR before following the external project's quick-start instructions.

The package provides:

- go-to pose control;
- waypoint following;
- circle and spiral path following;
- generic trajectory following;
- stop and emergency-stop services;
- direct six-thruster BlueROV2 allocation;
- optional model-aided virtual-wrench reconciliation diagnostics.

The controller is designed to run with the DAVE BlueROV2 simulator in direct-control mode:

```bash
ros2 launch dave_demos dave_robot.launch.py \
  z:=-0.5 \
  namespace:=bluerov2 \
  world_name:=dave_ocean_waves \
  paused:=false \
  use_ardusub:=false \
  use_teleop:=false

ros2 launch bluerov2_path_control path_controller.launch.py \
  model_name:=bluerov2 \
  use_bridge:=true
```

With the compatibility fixes applied, a DAVE smoke test confirmed Odometry input, an accepted
`GoTo` request, and non-zero commands on all six thruster topics.

The package is maintained externally by Waseem Akram at MARVIS LAB:

[MARVIS LAB](https://drwa92.github.io/marvis-lab/)

Project webpage:

[BlueROV2 controller project page](https://drwa92.github.io/bluerov2_reconciled_path_control_ros2/)
