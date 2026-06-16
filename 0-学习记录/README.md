# ROS2 学习记录

这个目录记录 ROS2 学习项目从环境搭建、基础通信、仿真控制到 Navigation2 的全过程。每次新增功能，都同步记录：做了什么、怎么跑、踩了什么坑、验证结果是什么。

## 主线笔记

建议按这个顺序阅读：

1. `01-环境搭建-WSL2-Ubuntu2404.md`
2. `02-ROS2-Jazzy安装.md`
3. `03-ROS2机器人开发基础.md`
4. `04-TurtleBot3基础操控.md`
5. `05-话题通信深入-两轮差速机器人项目.md`
6. `06-服务与参数管理.md`
7. `07-导航与运动指令.md`
8. `08-阶段总结与课程源码对比.md`
9. `09-面试用项目关键笔记.md`
10. `10-课程源码跑通记录.md`

## 辅助记录

- `命令记录.md`: 可以直接复跑的命令。
- `坑点记录.md`: 报错现象、原因、解决方案。
- `daily/`: 每日推进记录。
- `notebooks/`: 实验型笔记或 Jupyter Notebook。
- `screenshots/`: WSLg、RViz、Gazebo、TurtleBot3 等 GUI 运行截图。
- `阶段总结-从WSL到ROS2开发基础.md`: 早期阶段总结。
- `09-面试用项目关键笔记.md`: 面试口述、架构设计、关键技能、常见追问和简历写法。
- `10-课程源码跑通记录.md`: 课程源码编译、烟测、命令和坑点记录。

## 当前源码

项目源码统一放在：

```text
ros2-src/
```

当前已有：

- `ros2-src/differential_drive_robot`: 从 0 实现两轮差速机器人，包含话题、服务、参数、Gazebo、RViz。
- `ros2-src/nav2_cpp_tutorial`: Navigation2 C++ action 客户端，包含 Gazebo + SLAM + Nav2 bringup。

实际运行开发工作区在 WSL：

```bash
/home/ubuntu/ros2_ws/src
```

## 外部课程源码

课程源码已下载到仓库外部：

```text
C:\Users\lcy\code\ros2\course-src
```

对应远端：

```text
http://gitlab.0voice.com/2503_vip/ros2-src.git
```

对比结论见：

```text
08-阶段总结与课程源码对比.md
```

## Git 版本规则

每次迭代都按这个流程：

```text
feature/<功能名> -> dev -> main
```

- `feature/*`: 单次功能或文档整理分支。
- `dev`: 集成分支。
- `main`: 稳定发布分支。

## 已安装的 Codex skills

2026-06-10 已安装：

- `pdf`: 读取和整理 `ros2-doc` 中的 PDF 课程材料。
- `jupyter-notebook`: 写实验型学习笔记。
- `screenshot`: 保存 GUI/仿真运行证据。

注意：新安装的 Codex skills 需要重启 Codex 后才会在后续会话里自动生效。
