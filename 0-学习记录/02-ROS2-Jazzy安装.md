# 02 ROS2 Jazzy 安装

日期：2026-06-10

目标：在 WSL2 Ubuntu 24.04 GUI 环境中安装 ROS 2 Jazzy Desktop，并验证命令行 demo、turtlesim、RViz。

## 当前结论

ROS 2 Jazzy Desktop 已安装成功，开发工具也已补齐。

已验证：

- 操作系统：`Ubuntu 24.04.4 LTS`，代号 `noble`。
- locale：`en_US.UTF-8`。
- ROS 软件源：清华 ROS2 镜像源，`https://mirrors.tuna.tsinghua.edu.cn/ros2/ubuntu noble main`。
- 已安装：`ros-jazzy-desktop`。
- 已安装开发工具：`ros-dev-tools`，包含 `colcon`、`rosdep`、`vcstool` 等。
- `~/.bashrc` 已添加：`source /opt/ros/jazzy/setup.bash`。
- `ROS_DISTRO=jazzy`。
- C++ talker 与 Python listener 通信成功。
- `turtlesim_node` 启动成功。
- `rviz2` 启动成功，OpenGL 日志显示 `OpenGl version: 4.5`。

## 操作系统检查

```bash
lsb_release -a
```

实际结果：

```text
Description: Ubuntu 24.04.4 LTS
Release:     24.04
Codename:    noble
```

备注：课程笔记里写的是 `Ubuntu 24.04.2 LTS`，当前系统升级后是 `24.04.4 LTS`，仍然是 ROS 2 Jazzy 支持的 Ubuntu 24.04 noble。

## 安装步骤

设置 UTF-8 locale：

```bash
sudo apt update
sudo apt install -y locales software-properties-common curl gnupg2
sudo locale-gen en_US en_US.UTF-8
sudo update-locale LC_ALL=en_US.UTF-8 LANG=en_US.UTF-8
```

启用 Universe：

```bash
sudo add-apt-repository -y universe
```

添加 ROS2 GPG key：

```bash
sudo rm -f /usr/share/keyrings/ros-archive-keyring.gpg
curl -sSL https://raw.githubusercontent.com/ros/rosdistro/master/ros.key \
  | sudo gpg --dearmor --yes -o /usr/share/keyrings/ros-archive-keyring.gpg
```

添加清华 ROS2 源：

```bash
echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/ros-archive-keyring.gpg] https://mirrors.tuna.tsinghua.edu.cn/ros2/ubuntu noble main" \
  | sudo tee /etc/apt/sources.list.d/ros2.list > /dev/null
```

更新并升级系统：

```bash
sudo apt update
sudo apt upgrade -y
```

升级后重启 WSL 发行版：

```powershell
wsl --terminate Ubuntu-24.04
```

安装 ROS2 Desktop：

```bash
sudo apt install -y ros-jazzy-desktop
```

安装开发工具：

```bash
sudo apt install -y ros-dev-tools
```

初始化 rosdep：

```bash
sudo rosdep init
rosdep update
```

设置环境变量：

```bash
echo "source /opt/ros/jazzy/setup.bash" >> ~/.bashrc
source ~/.bashrc
```

## 验证结果

环境变量：

```bash
printenv | grep -E '^(ROS_DISTRO|AMENT_PREFIX_PATH|DISPLAY|WAYLAND_DISPLAY)='
```

结果：

```text
ROS_DISTRO=jazzy
AMENT_PREFIX_PATH=/opt/ros/jazzy
DISPLAY=:0
WAYLAND_DISPLAY=wayland-0
```

talker/listener：

```bash
ros2 run demo_nodes_cpp talker
ros2 run demo_nodes_py listener
```

已看到：

```text
[talker]: Publishing: 'Hello World: 1'
[listener]: I heard: [Hello World: 3]
```

turtlesim：

```bash
ros2 run turtlesim turtlesim_node
```

已看到：

```text
[turtlesim]: Starting turtlesim with node name /turtlesim
[turtlesim]: Spawning turtle [turtle1]
```

RViz：

```bash
rviz2
```

已看到：

```text
[rviz2]: OpenGl version: 4.5 (GLSL 4.5)
```

## 后续手动练习

打开第一个 Ubuntu 终端：

```bash
ros2 run turtlesim turtlesim_node
```

打开第二个 Ubuntu 终端：

```bash
ros2 run turtlesim turtle_teleop_key
```

然后使用方向键控制小乌龟。

## 参考

- ROS 2 Jazzy 官方安装文档：https://docs.ros.org/en/jazzy/Installation/Ubuntu-Install-Debs.html
- 清华 ROS2 镜像帮助：https://mirrors.tuna.tsinghua.edu.cn/help/ros2/

