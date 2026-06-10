# differential_drive_robot 学习记录

本包对应 `3-话题通信深入.pdf`，目标是从 0 实现一个两轮差速机器人仿真项目。

## 话题链路

- `teleop_keyboard` 发布 `geometry_msgs/msg/Twist` 到 `/teleop_cmd`。
- `robot_controller` 订阅 `/teleop_cmd`，做限速和超时保护。
- `robot_controller` 发布 `geometry_msgs/msg/TwistStamped` 到 `/cmd_vel`。
- `ros_gz_bridge` 将 `/cmd_vel` 桥接到 Gazebo `cmd_vel`。
- Gazebo `DiffDrive` 插件发布 `odom`、`tf`。
- Gazebo `JointStatePublisher` 插件发布 `joint_states`。

## 常用命令

```bash
cd ~/ros2_ws
colcon build --symlink-install --packages-select differential_drive_robot
source install/setup.bash
ros2 launch differential_drive_robot gazebo_harmonic.launch.py rviz:=false
ros2 run differential_drive_robot teleop_keyboard
```
