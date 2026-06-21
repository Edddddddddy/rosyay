# 04 TurtleBot3 基础操控

日期：2026-06-11

参考教程：

- `ros2-doc/2-TurtleBot3基础操控/1-ros2环境搭建和核心基础-2.pdf`
- `ros2-doc/2-TurtleBot3基础操控/2-TurtleBot3_初步操控.pdf`

## 本次目标

把教程里的 TurtleBot3 初步操控流程在 WSL2 Ubuntu 24.04 + ROS2 Jazzy + Gazebo Harmonic 环境中跑起来，并记录能复现的命令、验证结果和坑点。

## 学习计划

1. 跑通 Gazebo Harmonic 和 ROS-GZ 示例。
2. 跑通 TurtleBot3 Gazebo world。
3. 验证传感器话题、里程计和速度控制。
4. 使用 Cartographer 做一次 SLAM 建图并保存地图。
5. 用保存的地图启动 Navigation2，验证定位和导航栈节点。
6. 后续手动练习 RViz 里的 2D Pose Estimate 和 2D Goal Pose。

## 已安装内容

添加 OSRF Gazebo 源：

```bash
sudo apt-get update
sudo apt-get install -y lsb-release gnupg curl
sudo curl -fsSL https://packages.osrfoundation.org/gazebo.gpg \
  -o /usr/share/keyrings/pkgs-osrf-archive-keyring.gpg
echo 'deb [arch=amd64 signed-by=/usr/share/keyrings/pkgs-osrf-archive-keyring.gpg] http://packages.osrfoundation.org/gazebo/ubuntu-stable noble main' \
  | sudo tee /etc/apt/sources.list.d/gazebo-stable.list > /dev/null
sudo apt-get update
```

安装 Gazebo、ROS-GZ、TurtleBot3、SLAM 和 Navigation2：

```bash
sudo apt-get install -y \
  gz-harmonic \
  ros-jazzy-ros-gz \
  ros-jazzy-turtlebot3 \
  ros-jazzy-turtlebot3-gazebo \
  ros-jazzy-turtlebot3-teleop \
  ros-jazzy-turtlebot3-cartographer \
  ros-jazzy-turtlebot3-navigation2 \
  ros-jazzy-navigation2 \
  ros-jazzy-nav2-bringup \
  ros-jazzy-slam-toolbox \
  ros-jazzy-dynamixel-sdk
```

版本验证：

```bash
gz sim --version
```

结果：

```text
Gazebo Sim, version 8.13.0
```

## 环境变量

已写入 `~/.bashrc`：

```bash
export TURTLEBOT3_MODEL=waffle
export ROS_DOMAIN_ID=30 #TURTLEBOT3
```

验证：

```bash
printenv TURTLEBOT3_MODEL
printenv ROS_DOMAIN_ID
```

结果：

```text
waffle
30
```

## Gazebo ROS-GZ 示例

教程先用 `ros_gz_sim_demos` 验证 Gazebo 与 ROS2 桥接。

启动 diff drive 示例：

```bash
ros2 launch ros_gz_sim_demos diff_drive.launch.py
```

检查到的话题：

```text
/model/vehicle_blue/cmd_vel
/model/vehicle_green/cmd_vel
```

`/model/vehicle_blue/cmd_vel` 类型是：

```text
geometry_msgs/msg/Twist
```

发布速度命令：

```bash
ros2 topic pub --rate 5 /model/vehicle_blue/cmd_vel geometry_msgs/msg/Twist \
  '{linear: {x: 0.8}, angular: {z: 0.4}}'
```

启动 RGBD camera bridge 示例：

```bash
ros2 launch ros_gz_sim_demos rgbd_camera_bridge.launch.py
```

检查到的话题：

```text
/camera/camera_info
/camera/image
/rgbd_camera/camera_info
/rgbd_camera/depth_image
/rgbd_camera/image
/rgbd_camera/points
```

结论：ROS-GZ 桥接可用，Gazebo 示例能把模型控制和相机数据桥接到 ROS2。

## TurtleBot3 仿真启动

启动 TurtleBot3 world：

```bash
ros2 launch turtlebot3_gazebo turtlebot3_world.launch.py
```

关键成功日志：

```text
urdf_file_name : turtlebot3_waffle.urdf
[ros_gz_sim]: Entity creation successful.
[ros_gz_bridge]: Creating ROS->GZ Bridge: [cmd_vel (geometry_msgs/msg/TwistStamped) -> cmd_vel (gz.msgs.Twist)]
```

启动后的核心节点：

```text
/robot_state_publisher
/ros_gz_bridge
/ros_gz_image
```

启动后的核心话题：

```text
/camera/camera_info
/camera/image_raw
/cmd_vel
/imu
/joint_states
/odom
/scan
/tf
/tf_static
```

传感器验证：

```bash
ros2 topic echo /scan --once
ros2 topic echo /camera/camera_info --once
```

结果：

- `/scan` 能收到 `LaserScan`，`frame_id` 是 `base_scan`。
- `/camera/camera_info` 能收到相机内参，`frame_id` 是 `camera_rgb_frame`，分辨率为 `1920x1080`。

## 控制 TurtleBot3 移动

教程里的键盘控制：

```bash
ros2 run turtlebot3_teleop teleop_keyboard
```

本次为了自动验证，直接向 `/cmd_vel` 发布速度命令。注意当前 TurtleBot3 bridge 中 `/cmd_vel` 类型是 `geometry_msgs/msg/TwistStamped`，不是普通 `Twist`。

```bash
ros2 topic pub --rate 10 /cmd_vel geometry_msgs/msg/TwistStamped \
  '{twist: {linear: {x: 0.18, y: 0.0, z: 0.0}, angular: {x: 0.0, y: 0.0, z: 0.4}}}'
```

`/odom` 验证结果：

```text
移动前：x ≈ 0, y ≈ 0, orientation.w ≈ 1.0
移动后：x ≈ 0.1898, y ≈ 0.0492, orientation.z ≈ 0.2348, orientation.w ≈ 0.9720
```

结论：机器人实体已生成，ROS2 速度话题能控制机器人移动。

## SLAM 建图

启动 Gazebo：

```bash
ros2 launch turtlebot3_gazebo turtlebot3_world.launch.py
```

启动 Cartographer：

```bash
ros2 launch turtlebot3_cartographer cartographer.launch.py use_sim_time:=True
```

移动机器人采集地图：

```bash
ros2 topic pub --rate 10 /cmd_vel geometry_msgs/msg/TwistStamped \
  '{twist: {linear: {x: 0.18, y: 0.0, z: 0.0}, angular: {x: 0.0, y: 0.0, z: 0.4}}}'
```

检查地图：

```bash
ros2 topic echo /map --once
```

本次地图结果：

```text
resolution: 0.05
width: 92
height: 114
```

保存地图：

```bash
ros2 run nav2_map_server map_saver_cli -f /home/ubuntu/tb3_slam_test
```

生成文件：

```text
/home/ubuntu/tb3_slam_test.yaml
/home/ubuntu/tb3_slam_test.pgm
```

保存日志显示：

```text
Map saved successfully
```

结论：Cartographer 能正常订阅仿真雷达和里程计，成功生成并保存地图。

## Navigation2 导航验证

启动 Gazebo：

```bash
ros2 launch turtlebot3_gazebo turtlebot3_world.launch.py
```

使用刚保存的地图启动 Navigation2：

```bash
ros2 launch turtlebot3_navigation2 navigation2.launch.py \
  use_sim_time:=True \
  map:=/home/ubuntu/tb3_slam_test.yaml
```

导航栈核心节点已启动：

```text
/amcl
/behavior_server
/bt_navigator
/controller_server
/lifecycle_manager_localization
/lifecycle_manager_navigation
/map_server
/planner_server
/velocity_smoother
/waypoint_follower
```

核心话题已出现：

```text
/amcl_pose
/cmd_vel
/cmd_vel_nav
/cmd_vel_smoothed
/goal_pose
/map
/map_updates
/plan
/scan
/tf
/tf_static
```

地图话题验证：

```text
frame_id: map
resolution: 0.05
width: 92
height: 114
```

一开始日志会提示：

```text
AMCL cannot publish a pose or update the transform. Please set the initial pose...
Timed out waiting for transform from base_link to map
```

这是正常流程：Navigation2 需要先给 AMCL 一个初始位姿。图形界面里用 RViz 的 `2D Pose Estimate` 设置；命令行验证时我用一个临时 `rclpy` 发布器向 `/initialpose` 发了初始位姿。

补发初始位姿后验证：

```bash
ros2 topic echo /amcl_pose --once
ros2 run tf2_ros tf2_echo map base_link
```

结果：

```text
/amcl_pose frame_id: map
map -> base_link Translation: [0.025, 0.004, 0.010]
map -> base_link Rotation: yaw ≈ -0.149 degree
```

Navigation2 最终日志：

```text
Managed nodes are active
```

结论：Navigation2 能用保存的地图启动，AMCL 初始定位后 `map -> base_link` 变换建立，导航栈进入 active 状态。

## 当前结论

已经完成：

- Gazebo Harmonic 安装和版本验证。
- ROS-GZ diff drive 示例验证。
- ROS-GZ RGBD camera bridge 示例验证。
- TurtleBot3 Gazebo world 启动。
- `/scan`、`/camera/camera_info`、`/odom` 等话题验证。
- `/cmd_vel` 控制 TurtleBot3 移动。
- Cartographer SLAM 建图。
- 使用 `map_saver_cli` 保存地图。
- 使用保存地图启动 Navigation2。
- 发布初始位姿后 AMCL 和 `map -> base_link` 变换验证成功。

下一步建议：

1. 手动进入 Ubuntu 桌面终端练习 `teleop_keyboard`。
2. 在 RViz 里练习 `2D Pose Estimate` 和 `2D Goal Pose`。
3. 观察 `/tf`、`/map`、`/amcl_pose`、`/plan` 的变化。
4. 再做一次更完整的导航目标点实验。

## 本次坑点

1. 教程里的 Gazebo 旧包名需要按 Jazzy/Harmonic 调整，实际使用 `gz-harmonic` 和 `ros-jazzy-ros-gz`。
2. 从 PowerShell 调 WSL 时，`$HOME`、`$t`、`$!`、`$(...)` 容易被 PowerShell 抢先展开。
3. `ros_gz_sim_demos diff_drive` 的速度话题是 `Twist`，但 TurtleBot3 的 `/cmd_vel` 是 `TwistStamped`。
4. Navigation2 启动后没有初始位姿时，`base_link -> map` 变换缺失是正常现象，需要在 RViz 设置 `2D Pose Estimate`。
5. 命令行直接用 `ros2 topic pub` 发复杂 YAML 容易被多层 shell 引号弄坏；复杂消息可以用小脚本发布。

## 2026-06-21 复测补充：`map` 和 `odom` 不要混用

本笔记包含两种不同运行场景：

- 单独运行差速机器人或 TurtleBot3 Gazebo：通常已有 `odom`，但没有 `map`。
- 启动 Cartographer、SLAM Toolbox、AMCL 或 Nav2 定位：定位系统才会建立
  `map -> odom`。

因此只启动独立差速机器人时，RViz Fixed Frame 应设置为 `odom`：

```bash
ros2 launch differential_drive_robot gazebo_harmonic.launch.py rviz_fixed_frame:=odom
ros2 run tf2_ros tf2_echo odom base_footprint
```

只有下面的检查能得到 `map -> base_link` 或 `map -> base_footprint` 时，才使用
`map`：

```bash
ros2 run tf2_ros tf2_echo map base_link
```

如果 RViz 报：

```text
Global Status: Error
Frame [map] does not exist
```

这不是 Gazebo 模型失败，而是当前系统没有任何节点发布 `map` 坐标系。切换为
`odom`，或者启动 SLAM/定位节点即可。

2026-06-21 实际复测确认：完整 Gazebo + RViz 启动成功，RViz 命令行包含
`-f odom`，实体创建成功，`/odom`、`/tf` 桥接成功，日志中没有
`Frame [map] does not exist`。
