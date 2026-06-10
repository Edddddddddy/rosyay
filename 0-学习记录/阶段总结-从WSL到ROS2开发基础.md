# 阶段总结：从 WSL2 到 ROS2 开发基础

日期：2026-06-11

## 目前做了什么

1. 搭好了 WSL2 Ubuntu 24.04 环境。
2. 配好了 GUI 能力，使用 WSLg 运行 Linux 图形程序。
3. 安装了 ROS 2 Jazzy Desktop。
4. 安装了 ROS 开发工具：`ros-dev-tools`、`colcon`、`rosdep`。
5. 创建了 ROS2 工作空间：`~/ros2_ws`。
6. 创建并跑通了 Python 功能包：`my_package`。
7. 创建并跑通了 C++ 功能包：`my_cpp_pkg`。
8. 克隆了 ROS2 示例仓库：`ros2/examples` 的 `jazzy` 分支。
9. 验证了 talker/listener、turtlesim、rviz2、自写 Python 节点、自写 C++ 节点。

## 当前环境状态

```bash
lsb_release -a
```

当前系统：

```text
Ubuntu 24.04.4 LTS
Codename: noble
```

ROS 环境：

```bash
env | grep -E '^(ROS_DISTRO|ROS_VERSION|AMENT_PREFIX_PATH|PYTHONPATH)='
```

关键结果：

```text
ROS_VERSION=2
ROS_DISTRO=jazzy
AMENT_PREFIX_PATH=/home/ubuntu/ros2_ws/install/my_cpp_pkg:/home/ubuntu/ros2_ws/install/my_package:/opt/ros/jazzy
```

GUI 验证：

```bash
xclock
rviz2
```

RViz 已成功启动，OpenGL 可用。

## 目录结构

工作空间：

```bash
~/ros2_ws
├── src
│   ├── my_package
│   ├── my_cpp_pkg
│   └── examples
├── build
├── install
└── log
```

查看命令：

```bash
tree --charset ascii -L 3 ~/ros2_ws
```

## 安装 ROS2 Jazzy 的核心步骤

设置编码：

```bash
sudo apt update
sudo apt install -y locales software-properties-common curl gnupg2
sudo locale-gen en_US en_US.UTF-8
sudo update-locale LC_ALL=en_US.UTF-8 LANG=en_US.UTF-8
```

添加 Universe：

```bash
sudo add-apt-repository -y universe
```

添加 ROS2 key：

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

安装：

```bash
sudo apt update
sudo apt upgrade -y
sudo apt install -y ros-jazzy-desktop
sudo apt install -y ros-dev-tools
```

配置环境：

```bash
echo "source /opt/ros/jazzy/setup.bash" >> ~/.bashrc
source ~/.bashrc
```

初始化 rosdep：

```bash
sudo rosdep init
rosdep update
```

## 工作空间与包

创建工作空间：

```bash
mkdir -p ~/ros2_ws/src
cd ~/ros2_ws
```

创建 Python 包：

```bash
cd ~/ros2_ws/src
ros2 pkg create --build-type ament_python my_package --dependencies rclpy std_msgs
```

创建 C++ 包：

```bash
cd ~/ros2_ws/src
ros2 pkg create --build-type ament_cmake my_cpp_pkg \
  --dependencies rclcpp std_msgs sensor_msgs geometry_msgs \
  --node-name my_node
```

克隆 examples：

```bash
cd ~/ros2_ws/src
git clone --branch jazzy --depth 1 https://github.com/ros2/examples.git
```

查看工作空间包：

```bash
cd ~/ros2_ws
colcon list
```

## 编译和运行

编译指定包：

```bash
cd ~/ros2_ws
colcon build --symlink-install --packages-select my_package
colcon build --symlink-install --packages-select my_cpp_pkg
```

加载工作空间：

```bash
source /opt/ros/jazzy/setup.bash
source ~/ros2_ws/install/setup.bash
```

永久加载工作空间：

```bash
echo 'if [ -f ~/ros2_ws/install/setup.bash ]; then source ~/ros2_ws/install/setup.bash; fi' >> ~/.bashrc
```

运行 Python 节点：

```bash
ros2 run my_package learning_talker
```

验证 Python 话题：

```bash
ros2 topic echo /learning_chatter --once
```

运行 C++ 节点：

```bash
ros2 run my_cpp_pkg my_node
```

验证 C++ 话题：

```bash
ros2 topic echo /cpp_chatter --once
```

测试 C++ 包：

```bash
cd ~/ros2_ws
colcon test --packages-select my_cpp_pkg
colcon test-result --all
```

## 已验证成功的内容

ROS 官方 demo：

```bash
ros2 run demo_nodes_cpp talker
ros2 run demo_nodes_py listener
```

turtlesim：

```bash
ros2 run turtlesim turtlesim_node
ros2 run turtlesim turtle_teleop_key
```

RViz：

```bash
rviz2
```

自写 Python 节点：

```text
[learning_talker]: Publishing: "hello ros2 dev flow #1"
data: 'hello ros2 dev flow #3'
```

自写 C++ 节点：

```text
[my_node]: Publishing: "hello cpp package flow #1"
data: 'hello cpp package flow #3'
```

测试结果：

```text
Summary: 8 tests, 0 errors, 0 failures, 1 skipped
```

## 踩过的坑

### 1. WSL2 后端没启用

现象：

```text
WslRegisterDistribution failed with error: 0x80370114
HCS_E_SERVICE_NOT_AVAILABLE
```

处理：

管理员 PowerShell 中执行：

```powershell
dism.exe /online /enable-feature /featurename:Microsoft-Windows-Subsystem-Linux /all /norestart
dism.exe /online /enable-feature /featurename:VirtualMachinePlatform /all /norestart
bcdedit /set hypervisorlaunchtype auto
wsl --update
```

然后重启 Windows。

### 2. Ubuntu 默认进 root

处理：

编辑 `/etc/wsl.conf`：

```ini
[boot]
systemd=true

[user]
default=ubuntu
```

然后：

```powershell
wsl --terminate Ubuntu-24.04
```

### 3. PowerShell 抢先展开 Linux 命令

问题符号：

```text
$HOME
$USER
$ROS_DISTRO
$!
$(...)
```

表现：

- `dpkg` 被 PowerShell 当成 Windows 命令执行。
- `ros2.list` 被写坏。
- 后台进程清理出现 `kill` / `wait` 提示。

经验：

- 复杂 Linux 命令尽量进入 Ubuntu 终端执行。
- 从 PowerShell 调 WSL 时，少用 `$()`，用 `env | grep ...` 替代。

### 4. ROS2 GPG key 缺失

现象：

```text
NO_PUBKEY F42ED6FBAB17C654
```

处理：

```bash
sudo rm -f /usr/share/keyrings/ros-archive-keyring.gpg
curl -sSL https://raw.githubusercontent.com/ros/rosdistro/master/ros.key \
  | sudo gpg --dearmor --yes -o /usr/share/keyrings/ros-archive-keyring.gpg
sudo apt update
```

### 5. `tree` 显示乱码

处理：

```bash
tree --charset ascii -L 3 ~/ros2_ws
```

### 6. 不要直接全量编译 examples

原因：

- `ros2/examples` 里很多包和 `ros-jazzy-desktop` 已安装示例包同名。
- 初学阶段容易混淆 underlay / overlay。

建议：

```bash
colcon build --symlink-install --packages-select my_cpp_pkg
```

### 7. 反斜杠命令写法

正确：

```bash
ros2 pkg create --build-type ament_cmake my_cpp_pkg \
  --dependencies rclcpp std_msgs \
  --node-name my_node
```

不要写成：

```bash
ros2 pkg create --build-type ament_cmake my_cpp_pkg \--dependencies rclcpp std_msgs
```

### 8. 清理命令要谨慎

危险命令：

```bash
rm -rf build/ install/ log/
```

执行前确认：

```bash
pwd
```

应显示：

```text
/home/ubuntu/ros2_ws
```

## 常用命令速查

```bash
ros2 pkg list
ros2 pkg executables my_cpp_pkg
ros2 node list
ros2 topic list
ros2 topic echo /cpp_chatter
ros2 topic info /cpp_chatter
ros2 interface show std_msgs/msg/String
colcon list
colcon build --symlink-install --packages-select my_cpp_pkg
colcon test --packages-select my_cpp_pkg
colcon test-result --all
```

## 下一步

1. 给 `my_cpp_pkg` 增加 C++ subscriber 节点。
2. 学习 `ros2 node`、`ros2 topic`、`ros2 interface`。
3. 再进入 service、parameter、launch。
4. 最后接 TurtleBot3 和 Gazebo/RViz 仿真。

