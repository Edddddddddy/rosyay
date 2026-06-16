# 11-本地与 GitHub 结构整理

日期：2026-06-16

## 当前结论

这个学习项目现在分成三类区域：

```text
笔记资料区：ros2-doc/0-学习记录、1-* 到 5-*
自己代码区：ros2-doc/ros2-src
运行工作区：WSL /home/ubuntu/ros2_ws/src
```

真正进入 GitHub 版本控制的项目代码是：

```text
C:\Users\lcy\code\ros2\ros2-doc\ros2-src
```

GitHub 对应页面：

```text
https://github.com/Edddddddddy/rosyay/tree/main/ros2-src
```

## 本地文件结构

```text
C:\Users\lcy\code\ros2\
├── ros2-doc\
│   ├── .github\workflows\ros2-src-quality.yml
│   ├── .editorconfig
│   ├── .gitattributes
│   ├── .gitignore
│   ├── README.md
│   ├── 0-学习记录\
│   │   ├── README.md
│   │   ├── 命令记录.md
│   │   ├── 坑点记录.md
│   │   ├── daily\
│   │   └── 01-*.md 到 11-*.md
│   ├── 1-ros2环境搭建和核心基础\
│   ├── 2-TurtleBot3基础操控\
│   ├── 3-话题通信深入\
│   ├── 4-服务与参数管理\
│   ├── 5-导航与运动指令\
│   ├── tools\
│   │   └── check_ros2_src_quality.sh
│   └── ros2-src\
│       ├── README.md
│       ├── QUALITY.md
│       ├── .clang-format
│       ├── differential_drive_robot\
│       └── nav2_cpp_tutorial\
```

## WSL 运行结构

```text
/home/ubuntu/ros2_ws/
├── src/
│   ├── differential_drive_robot/
│   ├── nav2_cpp_tutorial/
│   ├── my_cpp_pkg/
│   ├── my_package/
│   └── examples/
├── build/
├── install/
└── log/
```

说明：

- `/home/ubuntu/ros2_ws/src` 用来编译和运行。
- `build/`、`install/`、`log/` 是构建产物，不进 Git。
- 当前 Git 代码区和 WSL 运行区里的 `differential_drive_robot`、`nav2_cpp_tutorial` 内容一致。

## 远端结构

远端仓库：

```text
https://github.com/Edddddddddy/rosyay
```

主要分支：

```text
main      稳定展示分支
dev       集成分支
feature/* 单次迭代分支
```

远端 `ros2-src` 只保存自己的项目包：

```text
ros2-src/
├── README.md
├── QUALITY.md
├── .clang-format
├── differential_drive_robot/
└── nav2_cpp_tutorial/
```

课程源码 `/home/ubuntu/ros2-src` 不直接提交到 GitHub 主项目。

## 已增加的控制项

### 1. 代码区入口 README

新增：

```text
ros2-src/README.md
```

作用：

- 说明 `ros2-src` 只放自己的 ROS2 项目代码。
- 解释本地 Git 目录、WSL 运行目录、课程源码目录的边界。
- 列出当前包、构建命令、测试命令、版本控制规则。

### 2. 版本和格式规范

新增：

```text
ros2-src/QUALITY.md
```

作用：

- 固化 `feature -> dev -> main` 流程。
- 规定 ROS2 包结构。
- 规定命名、缩进、行尾、构建产物处理规则。

### 3. 编辑器格式控制

新增：

```text
.editorconfig
ros2-src/.clang-format
```

作用：

- 统一 UTF-8、LF、末尾换行。
- C++ 使用 2 空格缩进。
- Python 使用 4 空格缩进。
- YAML/XML/URDF/SDF/RViz 使用 2 空格缩进。

### 4. 本地质量检查脚本

新增：

```text
tools/check_ros2_src_quality.sh
```

运行：

```bash
bash tools/check_ros2_src_quality.sh
```

检查：

- `ros2-src` 里没有 `build/`、`install/`、`log/`、`.colcon/`。
- 每个包包含 `CMakeLists.txt`、`package.xml`、`README.md`、`src/`。
- `package.xml` 的 `<name>` 与目录名一致。
- 文本文件无 CRLF。
- 文本文件无尾随空白。

### 5. GitHub Actions

新增：

```text
.github/workflows/ros2-src-quality.yml
```

作用：

- push 到 `main`、`dev`、`feature/**` 时自动检查。
- PR 时自动检查。
- 本地和远端使用同一个脚本，避免规则不一致。

## 后续使用规则

每次新增或修改代码：

```bash
git checkout dev
git pull --ff-only github dev
git checkout -b feature/<topic>

# 修改代码或笔记
bash tools/check_ros2_src_quality.sh

git add <files>
git commit -m "<summary>"
git push -u github feature/<topic>
```

确认可用后：

```bash
git checkout dev
git merge --no-ff feature/<topic> -m "Merge <topic> into dev"
git push github dev

git checkout main
git merge --no-ff dev -m "Release <topic>"
git push github main
```

## 一句话记忆

```text
笔记讲清楚，代码放 ros2-src，运行在 WSL，生成物不进 Git，所有迭代走 feature -> dev -> main。
```
