# ros2-src 版本与格式控制

## 代码和笔记边界

当前仓库同时保存课程资料、学习笔记和自己实现的 ROS2 项目。边界如下：

| 区域 | 路径 | 是否提交 GitHub | 用途 |
| --- | --- | --- | --- |
| 学习笔记 | `0-学习记录/` | 是 | 总结、命令、坑点、阶段复盘。 |
| 课程资料 | `1-*` 到 `5-*` | 是 | PDF 和课程原始资料。 |
| 自己项目代码 | `ros2-src/` | 是 | 真正纳入版本控制的 ROS2 包。 |
| WSL 编译运行区 | `/home/ubuntu/ros2_ws/src` | 否 | ROS2 实际构建和运行。 |
| 课程源码参考区 | `/home/ubuntu/ros2-src` | 否 | 课程源码复现和对比。 |

## 分支流程

每次迭代使用：

```text
feature/<topic> -> dev -> main
```

推荐命令：

```bash
git checkout dev
git pull --ff-only github dev
git checkout -b feature/<topic>

# 修改、验证、提交
git add <files>
git commit -m "<short summary>"
git push -u github feature/<topic>

# 集成到 dev
git checkout dev
git pull --ff-only github dev
git merge --no-ff feature/<topic> -m "Merge <topic> into dev"
git push github dev

# 发布到 main
git checkout main
git pull --ff-only github main
git merge --no-ff dev -m "Release <topic>"
git push github main
```

## 包结构要求

每个 `ros2-src/*` 包至少包含：

```text
CMakeLists.txt
package.xml
README.md
src/
```

推荐包含：

```text
include/<package_name>/   # 公共头文件
config/                   # 参数和桥接配置
launch/                   # launch.py
rviz/                     # RViz 配置
urdf/                     # 机器人模型
worlds/                   # Gazebo world
srv/                      # service 接口
doc/                      # 包内设计笔记
```

## 命名规则

- 包名：小写 snake_case，如 `differential_drive_robot`。
- 可执行文件：小写 snake_case，如 `robot_controller`。
- C++ 类：UpperCamelCase，如 `RobotController`。
- 服务/动作接口：UpperCamelCase，如 `SetControlMode.srv`。
- 参数名、话题名：小写 snake_case。

## 格式规则

- 文本文件：UTF-8、LF、文件末尾保留换行。
- C++：2 空格缩进，`clang-format` 配置见 `ros2-src/.clang-format`。
- Python：4 空格缩进。
- YAML/XML/URDF/SDF/RViz：2 空格缩进。
- 不提交尾随空白。
- 不提交生成目录：`build/`、`install/`、`log/`、`.colcon/`。

## 本地质量检查

在仓库根目录运行：

```bash
bash tools/check_ros2_src_quality.sh
```

检查内容：

- `ros2-src` 内没有提交构建产物目录。
- 每个 ROS2 包有 `CMakeLists.txt`、`package.xml`、`README.md`、`src/`。
- `package.xml` 中的 `<name>` 与目录名一致。
- 关键文本文件无 CRLF。
- 关键文本文件无尾随空白。

## GitHub 远端控制

已添加 GitHub Actions：

```text
.github/workflows/ros2-src-quality.yml
```

它会在 `main`、`dev`、`feature/**` 分支 push 或 PR 时运行同一个本地质量检查脚本。这样本地和远端使用同一套项目卫生规则。
