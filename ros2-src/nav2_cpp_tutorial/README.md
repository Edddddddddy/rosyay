# nav2_cpp_tutorial

C++ Navigation2 action client tutorial for ROS2 Jazzy.

This package starts TurtleBot3 Gazebo, SLAM Toolbox, and Navigation2, then uses a
C++ `NavigateToPose` action client to send navigation goals.

## Build

```bash
cd ~/ros2_ws
colcon build --symlink-install --packages-select nav2_cpp_tutorial
source install/setup.bash
```

## Run Demo

```bash
export TURTLEBOT3_MODEL=waffle
ros2 launch nav2_cpp_tutorial nav2_cpp_demo.launch.py
```

Run the C++ test after Nav2 is ready:

```bash
ros2 run nav2_cpp_tutorial nav2_simple_test
```

Or start the launch file with delayed test execution:

```bash
ros2 launch nav2_cpp_tutorial nav2_cpp_demo.launch.py run_test:=true
```

## Smoke Check

```bash
ros2 node list | sort
ros2 topic list | grep -E '^/(cmd_vel|cmd_vel_nav|cmd_vel_smoothed|map|odom|scan|tf|tf_static)$'
```

The launch is healthy when both lifecycle managers report:

```text
lifecycle_manager_slam: Managed nodes are active
lifecycle_manager_navigation: Managed nodes are active
```

## Jazzy Notes

Navigation2 Jazzy starts nodes such as `velocity_smoother`,
`collision_monitor`, and `docking_server` from the default bringup launch.
Keep their parameter blocks in `config/nav2_params.yaml`, otherwise lifecycle
bringup can fail before the action server becomes usable.
