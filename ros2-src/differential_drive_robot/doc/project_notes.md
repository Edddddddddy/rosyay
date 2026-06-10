# differential_drive_robot 学习记录

本包对应 `3-话题通信深入.pdf`，目标是从 0 实现一个两轮差速机器人仿真项目。

## 话题链路

- `teleop_keyboard` 发布 `geometry_msgs/msg/TwistStamped` 到 `/teleop_cmd`。
- `teleop_keyboard` 发布 `std_msgs/msg/String` 到 `/control_mode`。
- `robot_controller` 订阅 `/teleop_cmd` 和 `/control_mode`，做限速、速度平滑和超时保护。
- `robot_controller` 提供 `/set_control_mode` 服务，用于切换控制模式。
- `robot_controller` 发布 `geometry_msgs/msg/TwistStamped` 到 `/cmd_vel`。
- `ros_gz_bridge` 将 `/cmd_vel` 桥接到 Gazebo `cmd_vel`。
- Gazebo `DiffDrive` 插件发布 `odom`、`tf`。
- Gazebo `JointStatePublisher` 插件发布 `joint_states`。

## 控制模式

- `manual`
- `auto_forward`
- `auto_circle`
- `obstacle_avoidance`
- `wall_following`

## 常用命令

```bash
cd ~/ros2_ws
colcon build --symlink-install --packages-select differential_drive_robot
source install/setup.bash
ros2 launch differential_drive_robot gazebo_harmonic.launch.py rviz:=false
ros2 run differential_drive_robot teleop_keyboard
```
