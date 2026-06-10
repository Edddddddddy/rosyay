from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.actions import ExecuteProcess
from launch.actions import IncludeLaunchDescription
from launch.actions import TimerAction
from launch.conditions import IfCondition
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare


def generate_launch_description():
    use_sim_time = LaunchConfiguration('use_sim_time')
    slam = LaunchConfiguration('slam')
    rviz = LaunchConfiguration('rviz')
    run_test = LaunchConfiguration('run_test')

    pkg_turtlebot3_gazebo = FindPackageShare('turtlebot3_gazebo')
    pkg_nav2_bringup = FindPackageShare('nav2_bringup')
    pkg_this = FindPackageShare('nav2_cpp_tutorial')

    gazebo_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource([
            PathJoinSubstitution([
                pkg_turtlebot3_gazebo,
                'launch',
                'turtlebot3_world.launch.py',
            ]),
        ])
    )

    nav2_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource([
            PathJoinSubstitution([pkg_nav2_bringup, 'launch', 'bringup_launch.py']),
        ]),
        launch_arguments={
            'slam': slam,
            'use_sim_time': use_sim_time,
            'params_file': PathJoinSubstitution([pkg_this, 'config', 'nav2_params.yaml']),
            'use_rviz': rviz,
        }.items(),
    )

    test_node = TimerAction(
        period=15.0,
        actions=[
            Node(
                package='nav2_cpp_tutorial',
                executable='nav2_simple_test',
                name='nav2_cpp_test',
                output='screen',
                condition=IfCondition(run_test),
            ),
        ],
    )

    return LaunchDescription([
        DeclareLaunchArgument('use_sim_time', default_value='True'),
        DeclareLaunchArgument('slam', default_value='True'),
        DeclareLaunchArgument('rviz', default_value='True'),
        DeclareLaunchArgument('run_test', default_value='False'),
        gazebo_launch,
        nav2_launch,
        test_node,
        ExecuteProcess(
            cmd=['echo', 'Navigation2 C++ demo launch started'],
            output='screen',
        ),
    ])
