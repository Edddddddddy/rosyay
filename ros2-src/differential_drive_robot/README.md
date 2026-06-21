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

The standalone simulation publishes `odom`, not `map`, so RViz uses `odom` as
its fixed frame by default. To override it after starting SLAM or localization:

```bash
ros2 launch differential_drive_robot gazebo_harmonic.launch.py rviz_fixed_frame:=map
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

- `w`: faster forward
- `x`: faster reverse
- `a`: turn left
- `d`: turn right
- `s` or space: stop
- `1`: manual mode
- `2`: auto forward
- `3`: auto circle
- `4`: obstacle avoidance
- `0`: wall following
- `q`: quit

Mode service:

```bash
source ~/ros2_ws/install/setup.bash
ros2 service list -t | grep set_control_mode
ros2 service call /set_control_mode differential_drive_robot/srv/SetControlMode "{mode: auto_circle}"
```

Run the command as one line in an Ubuntu/WSL Bash terminal. PowerShell does not
use `\` as a line-continuation character.
