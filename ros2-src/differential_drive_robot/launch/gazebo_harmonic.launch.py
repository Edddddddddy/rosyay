import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.conditions import IfCondition
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():
    pkg_share = get_package_share_directory('differential_drive_robot')
    ros_gz_sim = get_package_share_directory('ros_gz_sim')

    urdf_file = os.path.join(pkg_share, 'urdf', 'robot_harmonic.urdf')
    world_file = os.path.join(pkg_share, 'worlds', 'empty_world.sdf')
    bridge_file = os.path.join(pkg_share, 'config', 'bridge.yaml')
    controller_params = os.path.join(pkg_share, 'config', 'controller_params.yaml')
    rviz_file = os.path.join(pkg_share, 'rviz', 'robot_view.rviz')

    with open(urdf_file, 'r', encoding='utf-8') as infp:
        robot_description = infp.read()

    use_sim_time = LaunchConfiguration('use_sim_time')
    rviz = LaunchConfiguration('rviz')

    gazebo = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(ros_gz_sim, 'launch', 'gz_sim.launch.py')),
        launch_arguments={
            'gz_args': f'-r -v2 {world_file}',
            'on_exit_shutdown': 'true',
        }.items(),
    )

    bridge = Node(
        package='ros_gz_bridge',
        executable='parameter_bridge',
        arguments=[
            '--ros-args',
            '-p',
            f'config_file:={bridge_file}',
        ],
        output='screen',
    )

    robot_state_publisher = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        name='robot_state_publisher',
        output='screen',
        parameters=[
            {'use_sim_time': use_sim_time},
            {'robot_description': robot_description},
        ],
    )

    spawn_robot = Node(
        package='ros_gz_sim',
        executable='create',
        arguments=[
            '-name', 'differential_drive_robot',
            '-file', urdf_file,
            '-x', '0.0',
            '-y', '0.0',
            '-z', '0.08',
        ],
        output='screen',
    )

    controller = Node(
        package='differential_drive_robot',
        executable='robot_controller',
        name='robot_controller',
        output='screen',
        parameters=[controller_params, {'use_sim_time': use_sim_time}],
    )

    rviz_node = Node(
        package='rviz2',
        executable='rviz2',
        name='rviz2',
        arguments=['-d', rviz_file],
        condition=IfCondition(rviz),
        parameters=[{'use_sim_time': use_sim_time}],
        output='screen',
    )

    return LaunchDescription([
        DeclareLaunchArgument('use_sim_time', default_value='true'),
        DeclareLaunchArgument('rviz', default_value='true'),
        gazebo,
        bridge,
        robot_state_publisher,
        spawn_robot,
        controller,
        rviz_node,
    ])
