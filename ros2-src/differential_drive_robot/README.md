# differential_drive_robot

A ROS 2 Jazzy learning project that builds a two-wheel differential drive robot from scratch.

## Build

```bash
cd ~/ros2_ws
colcon build --symlink-install --packages-select differential_drive_robot
source install/setup.bash
```

## Run

```bash
ros2 launch differential_drive_robot gazebo_harmonic.launch.py
```

For command-line smoke tests without RViz:

```bash
ros2 launch differential_drive_robot gazebo_harmonic.launch.py rviz:=false
```

## Teleop

Open another terminal:

```bash
source ~/ros2_ws/install/setup.bash
ros2 run differential_drive_robot teleop_keyboard
```

Keys:

- `i`: faster forward
- `,`: faster reverse
- `j`: turn left
- `l`: turn right
- `k` or space: stop
- `q`: quit
