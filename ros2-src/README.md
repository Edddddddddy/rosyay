# ros2-src

这里存放自己实现并纳入 GitHub 版本控制的 ROS2 练习项目。课程源码、构建产物和临时实验不要放进这个目录。

GitHub 入口：

```text
https://github.com/Edddddddddy/rosyay/tree/main/ros2-src
```

## 当前包

| 包 | 类型 | 作用 |
| --- | --- | --- |
| `differential_drive_robot` | `ament_cmake` | 从 0 实现两轮差速机器人，包含 Gazebo、RViz、话题、服务、参数和基础避障/跟墙控制。 |
| `nav2_cpp_tutorial` | `ament_cmake` | Navigation2 C++ action client 学习包，启动 TurtleBot3 Gazebo + SLAM + Nav2，并发送 `NavigateToPose` 目标。 |

## 本地结构

```text
C:\Users\lcy\code\ros2\
├── ros2-doc\                    # GitHub 仓库 rosyay，本地版本控制根目录
│   ├── 0-学习记录\              # 学习笔记、命令记录、坑点记录
│   ├── 1-ros2环境搭建和核心基础\  # 课程资料
│   ├── 2-TurtleBot3基础操控\     # 课程资料
│   ├── 3-话题通信深入\           # 课程资料
│   ├── 4-服务与参数管理\         # 课程资料
│   ├── 5-导航与运动指令\         # 课程资料
│   └── ros2-src\                # 自己实现并提交 GitHub 的 ROS2 项目代码
```

WSL 实际编译运行区：

```text
/home/ubuntu/ros2_ws/src
```

规则：`ros2-doc/ros2-src` 是代码版本控制源，`/home/ubuntu/ros2_ws/src` 是运行工作区。修改代码后要确保两个位置同步，再提交 Git。

课程源码参考区：

```text
/home/ubuntu/ros2-src
```

规则：课程源码只作为参考和复现，不直接并入 `ros2-doc/ros2-src`，也不提交其 `build/`、`install/`、`log/`。

## 推荐包结构

每个 C++ ROS2 包尽量保持：

```text
package_name/
├── CMakeLists.txt
├── package.xml
├── README.md
├── include/package_name/      # 对外头文件，可选
├── src/                       # C++ 节点和库实现
├── config/                    # 参数 YAML、bridge 配置
├── launch/                    # launch.py
├── rviz/                      # RViz 配置，可选
├── urdf/                      # 机器人模型，可选
├── worlds/                    # Gazebo world，可选
├── srv/                       # 自定义 service，可选
└── doc/                       # 包内设计笔记，可选
```

## 构建

```bash
source /opt/ros/jazzy/setup.bash
cd ~/ros2_ws
colcon build --symlink-install --packages-select differential_drive_robot
colcon build --symlink-install --packages-select nav2_cpp_tutorial
source install/setup.bash
```

全量构建：

```bash
cd ~/ros2_ws
colcon build --symlink-install
```

## 测试

```bash
cd ~/ros2_ws
colcon test --packages-select differential_drive_robot
colcon test --packages-select nav2_cpp_tutorial
colcon test-result --all
```

项目结构和格式检查：

```bash
bash tools/check_ros2_src_quality.sh
```

## 版本控制规则

固定使用：

```text
feature/<topic> -> dev -> main
```

- `feature/*`：单次功能、文档或整理任务。
- `dev`：集成分支，确认构建和文档没问题后合入。
- `main`：稳定展示分支，对应 GitHub 默认阅读入口。

不要提交：

- `build/`
- `install/`
- `log/`
- `.colcon/`
- 课程源码整包
- 账号密码、token、私钥

## 格式规则

- 文本统一 UTF-8 + LF。
- C++ 使用 2 空格缩进，优先遵循 ROS2/ament 风格。
- Python launch 文件使用 4 空格缩进。
- YAML/XML/URDF/SDF/RViz 使用 2 空格缩进。
- 参数放在 `config/`，启动逻辑放在 `launch/`，不要在源码中硬编码本机绝对路径。
- 新包必须包含 `README.md`，说明构建、运行和验证命令。
