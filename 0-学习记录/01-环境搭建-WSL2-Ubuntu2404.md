# 01 环境搭建：WSL2 + Ubuntu 24.04

日期：2026-06-10

目标：为 ROS2 学习项目准备 WSL2 Ubuntu 24.04 环境，并验证命令行与 GUI 能力。

## 当前结论

Ubuntu 24.04 LTS 已经注册为 WSL2 发行版，并且 WSLg GUI 通道可用。当前采用的是 WSLg 方式运行 Linux GUI 应用，不是安装完整 Ubuntu Desktop 桌面环境。

已经确认的状态：

- Windows：Windows 11 Pro for Workstations 10.0.26200，64-bit。
- CPU 虚拟化固件：已开启，`VirtualizationFirmwareEnabled=True`。
- WSL：已安装，`wsl --version` 显示 WSL 2.7.3.0，WSLg 1.0.73。
- 已安装 Ubuntu 24.04 应用包：`CanonicalGroupLimited.Ubuntu24.04LTS_2404.0.5.0_x64__79rhkp1fndgsc`，状态 `Ok`。
- `ubuntu2404.exe` 已存在于 WindowsApps。
- `wsl --list --verbose` 已显示 `Ubuntu-24.04`，版本为 2。
- 默认用户已调整为 `ubuntu`。
- `/etc/wsl.conf` 已启用 systemd，并设置默认用户。
- GUI 环境变量正常：`DISPLAY=:0`，`WAYLAND_DISPLAY=wayland-0`。
- 已安装 GUI 测试工具：`x11-apps`、`mesa-utils`。
- `glxinfo -B` 显示 Direct Rendering 可用，OpenGL renderer 为 `D3D12 (NVIDIA GeForce RTX 3060 Ti)`。

判断：当前环境已经满足后续运行 ROS2 GUI 工具的基础条件，例如 RViz、rqt、Gazebo 这类窗口程序。

## 已执行步骤

1. 查看 WSL 状态与在线发行版列表。
2. 通过 `wsl --install` 尝试安装 `Ubuntu-24.04`，但未完成发行版注册。
3. 通过 `winget` 安装 Ubuntu 24.04 LTS 应用包，安装成功。
4. 尝试启动 `ubuntu2404.exe` 注册发行版，失败于 `0x80370114`。
5. 检查 CPU 虚拟化状态，确认 BIOS/固件虚拟化已开启。
6. Windows 侧 WSL2 后端恢复后，`Ubuntu-24.04` 注册并启动成功。
7. 设置默认用户为 `ubuntu`。
8. 安装 `x11-apps`、`mesa-utils` 并验证 WSLg/OpenGL。

## 曾经需要管理员权限执行

用“管理员身份运行 PowerShell”，执行：

```powershell
dism.exe /online /enable-feature /featurename:Microsoft-Windows-Subsystem-Linux /all /norestart
dism.exe /online /enable-feature /featurename:VirtualMachinePlatform /all /norestart
bcdedit /set hypervisorlaunchtype auto
wsl --update
```

然后重启 Windows。该问题已处理完成。

重启后再执行：

```powershell
ubuntu2404.exe
```

完成后验证：

```powershell
wsl -d Ubuntu-24.04 -- lsb_release -a
wsl -d Ubuntu-24.04 -- uname -a
wsl -d Ubuntu-24.04 -- bash -lc 'echo WAYLAND_DISPLAY=$WAYLAND_DISPLAY; echo DISPLAY=$DISPLAY'
```

## GUI/桌面说明

WSL2 在 Windows 11 上通常通过 WSLg 运行 Linux GUI 应用，不一定需要安装完整 Ubuntu Desktop。对 ROS2 学习来说，优先目标是能运行 RViz、Gazebo、rqt 这类 GUI 程序。

WSL 能启动后，可以先安装轻量 GUI 测试包：

```bash
sudo apt update
sudo apt install -y x11-apps mesa-utils
xclock
glxinfo -B
```

如果 `xclock` 能弹窗，说明 WSLg GUI 通道可用。后续再安装 ROS2 Jazzy、TurtleBot3、RViz/Gazebo。

如果明确需要“完整 Ubuntu 桌面”，建议另外安装 `xfce4 + xrdp`，然后用 Windows 远程桌面连接 `localhost:<端口>`。但 ROS2 学习阶段优先使用 WSLg，依赖少、启动快，也更适合直接打开 RViz/Gazebo。
