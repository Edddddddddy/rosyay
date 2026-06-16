# rosyay

ROS2 Jazzy learning notes and practice projects.

## Contents

- `1-ros2环境搭建和核心基础/`: course PDFs and original notes.
- `2-TurtleBot3基础操控/`: TurtleBot3 setup and simulation material.
- `3-话题通信深入/`: topic communication course material.
- `4-服务与参数管理/`: service and parameter course material.
- `5-导航与运动指令/`: navigation and motion command material.
- `0-学习记录/`: my learning notes, command records, and troubleshooting notes.
- `ros2-src/`: ROS2 practice source packages implemented during learning.

## Current Project

`ros2-src/differential_drive_robot` is a from-scratch two-wheel differential drive robot project for ROS2 Jazzy + Gazebo Harmonic.

Build inside Ubuntu/WSL:

```bash
cd ~/ros2_ws
colcon build --symlink-install --packages-select differential_drive_robot
source install/setup.bash
```

Run:

```bash
ros2 launch differential_drive_robot gazebo_harmonic.launch.py
```

## Course Source Run-Through

Course source has also been built and smoke-tested in WSL:

```text
/home/ubuntu/ros2-src
```

Detailed notes:

```text
0-学习记录/10-课程源码跑通记录.md
```
