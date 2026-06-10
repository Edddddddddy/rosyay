cmake_minimum_required(VERSION 3.8)
project(differential_drive_robot)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(std_msgs REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(rosidl_default_generators REQUIRED)

rosidl_generate_interfaces(${PROJECT_NAME}
  "srv/SetControlMode.srv"
)

add_executable(robot_controller src/robot_controller.cpp)
ament_target_dependencies(robot_controller
  rclcpp
  geometry_msgs
  sensor_msgs
  nav_msgs
  std_msgs
)

add_executable(teleop_keyboard src/teleop_keyboard.cpp)
ament_target_dependencies(teleop_keyboard
  rclcpp
  geometry_msgs
  std_msgs
)

rosidl_get_typesupport_target(cpp_typesupport_target ${PROJECT_NAME} "rosidl_typesupport_cpp")
target_link_libraries(robot_controller "${cpp_typesupport_target}")

install(TARGETS
  robot_controller
  teleop_keyboard
  DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY
  config
  doc
  launch
  rviz
  srv
  urdf
  worlds
  DESTINATION share/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_export_dependencies(rosidl_default_runtime)
ament_package()
