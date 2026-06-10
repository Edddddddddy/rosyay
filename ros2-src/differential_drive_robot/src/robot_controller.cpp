#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <limits>
#include <memory>
#include <string>

#include "differential_drive_robot/srv/set_control_mode.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "geometry_msgs/msg/twist_stamped.hpp"
#include "nav_msgs/msg/odometry.hpp"
#include "rcl_interfaces/msg/set_parameters_result.hpp"
#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/joint_state.hpp"
#include "sensor_msgs/msg/laser_scan.hpp"
#include "std_msgs/msg/string.hpp"

using namespace std::chrono_literals;

class RobotController : public rclcpp::Node
{
public:
  RobotController()
  : Node("robot_controller")
  {
    load_parameters();

    cmd_pub_ = create_publisher<geometry_msgs::msg::TwistStamped>("cmd_vel", 10);

    teleop_sub_ = create_subscription<geometry_msgs::msg::TwistStamped>(
      "teleop_cmd", 10,
      [this](const geometry_msgs::msg::TwistStamped::SharedPtr msg) {
        last_teleop_ = msg->twist;
        last_cmd_time_ = now();
      });

    mode_sub_ = create_subscription<std_msgs::msg::String>(
      "control_mode", 10,
      [this](const std_msgs::msg::String::SharedPtr msg) {
        set_mode(msg->data);
      });

    odom_sub_ = create_subscription<nav_msgs::msg::Odometry>(
      "odom", 10,
      [this](const nav_msgs::msg::Odometry::SharedPtr msg) {
        last_odom_ = *msg;
        got_odom_ = true;
      });

    joint_state_sub_ = create_subscription<sensor_msgs::msg::JointState>(
      "joint_states", 10,
      [this](const sensor_msgs::msg::JointState::SharedPtr msg) {
        last_joint_state_count_ = msg->name.size();
      });

    scan_sub_ = create_subscription<sensor_msgs::msg::LaserScan>(
      "scan", 10,
      [this](const sensor_msgs::msg::LaserScan::SharedPtr msg) {
        last_scan_ = *msg;
        got_scan_ = true;
      });

    mode_srv_ = create_service<differential_drive_robot::srv::SetControlMode>(
      "set_control_mode",
      [this](
        const std::shared_ptr<differential_drive_robot::srv::SetControlMode::Request> request,
        std::shared_ptr<differential_drive_robot::srv::SetControlMode::Response> response) {
        response->success = set_mode(request->mode);
        response->message = response->success ? "mode changed to " + control_mode_ :
        "invalid mode: " + request->mode;
      });

    parameter_callback_handle_ = add_on_set_parameters_callback(
      [this](const std::vector<rclcpp::Parameter> & parameters) {
        return on_parameters(parameters);
      });

    const auto period = std::chrono::duration<double>(1.0 / control_rate_);
    control_timer_ = create_wall_timer(
      std::chrono::duration_cast<std::chrono::nanoseconds>(period),
      [this]() {control_loop();});
    status_timer_ = create_wall_timer(2s, [this]() {log_status();});

    RCLCPP_INFO(
      get_logger(),
      "robot_controller ready: modes manual/auto_forward/auto_circle/obstacle_avoidance/wall_following");
  }

private:
  void load_parameters()
  {
    max_linear_speed_ = declare_parameter("max_linear_speed", 0.26);
    max_angular_speed_ = declare_parameter("max_angular_speed", 1.82);
    max_linear_acceleration_ = declare_parameter("max_linear_acceleration", 0.5);
    max_angular_acceleration_ = declare_parameter("max_angular_acceleration", 1.0);
    command_timeout_ = declare_parameter("command_timeout", 0.7);
    obstacle_stop_distance_ = declare_parameter("obstacle_stop_distance", 0.45);
    wall_target_distance_ = declare_parameter("wall_target_distance", 0.55);
    wall_kp_ = declare_parameter("wall_kp", 1.0);
    control_rate_ = declare_parameter("control_rate", 20.0);
  }

  rcl_interfaces::msg::SetParametersResult on_parameters(
    const std::vector<rclcpp::Parameter> & parameters)
  {
    rcl_interfaces::msg::SetParametersResult result;
    result.successful = true;

    for (const auto & parameter : parameters) {
      const auto & name = parameter.get_name();
      if (name == "max_linear_speed") {
        max_linear_speed_ = parameter.as_double();
      } else if (name == "max_angular_speed") {
        max_angular_speed_ = parameter.as_double();
      } else if (name == "max_linear_acceleration") {
        max_linear_acceleration_ = parameter.as_double();
      } else if (name == "max_angular_acceleration") {
        max_angular_acceleration_ = parameter.as_double();
      } else if (name == "command_timeout") {
        command_timeout_ = parameter.as_double();
      } else if (name == "obstacle_stop_distance") {
        obstacle_stop_distance_ = parameter.as_double();
      } else if (name == "wall_target_distance") {
        wall_target_distance_ = parameter.as_double();
      } else if (name == "wall_kp") {
        wall_kp_ = parameter.as_double();
      }
    }

    return result;
  }

  bool set_mode(const std::string & requested_mode)
  {
    static const std::array<std::string, 5> valid_modes = {
      "manual", "auto_forward", "auto_circle", "obstacle_avoidance", "wall_following"};

    if (std::find(valid_modes.begin(), valid_modes.end(), requested_mode) == valid_modes.end()) {
      RCLCPP_WARN(get_logger(), "rejected invalid control mode: %s", requested_mode.c_str());
      return false;
    }

    if (control_mode_ != requested_mode) {
      control_mode_ = requested_mode;
      current_cmd_ = geometry_msgs::msg::Twist();
      target_cmd_ = geometry_msgs::msg::Twist();
      publish_velocity(current_cmd_);
      RCLCPP_INFO(get_logger(), "control mode changed to %s", control_mode_.c_str());
    }

    return true;
  }

  void control_loop()
  {
    target_cmd_ = select_target_command();

    if (!is_safe_to_move(target_cmd_)) {
      emergency_stop();
      return;
    }

    limit_velocities(target_cmd_);
    smooth_velocities();
    publish_velocity(current_cmd_);
  }

  geometry_msgs::msg::Twist select_target_command()
  {
    if (control_mode_ == "manual") {
      return manual_control();
    }
    if (control_mode_ == "auto_forward") {
      return auto_forward_control();
    }
    if (control_mode_ == "auto_circle") {
      return auto_circle_control();
    }
    if (control_mode_ == "obstacle_avoidance") {
      return obstacle_avoidance_control();
    }
    if (control_mode_ == "wall_following") {
      return wall_following_control();
    }
    return geometry_msgs::msg::Twist();
  }

  geometry_msgs::msg::Twist manual_control()
  {
    if (last_cmd_time_.nanoseconds() == 0) {
      return geometry_msgs::msg::Twist();
    }

    const double age = (now() - last_cmd_time_).seconds();
    if (age > command_timeout_) {
      return geometry_msgs::msg::Twist();
    }

    return last_teleop_;
  }

  geometry_msgs::msg::Twist auto_forward_control()
  {
    geometry_msgs::msg::Twist command;
    command.linear.x = 0.16;
    return command;
  }

  geometry_msgs::msg::Twist auto_circle_control()
  {
    geometry_msgs::msg::Twist command;
    command.linear.x = 0.14;
    command.angular.z = 0.45;
    return command;
  }

  geometry_msgs::msg::Twist obstacle_avoidance_control()
  {
    geometry_msgs::msg::Twist command;
    if (get_front_distance() < obstacle_stop_distance_) {
      command.angular.z = 0.75;
    } else {
      command.linear.x = 0.12;
    }
    return command;
  }

  geometry_msgs::msg::Twist wall_following_control()
  {
    geometry_msgs::msg::Twist command;
    command.linear.x = 0.10;

    if (!got_scan_) {
      return command;
    }

    const double front_distance = get_front_distance();
    const double right_distance = get_right_distance();
    if (front_distance < obstacle_stop_distance_) {
      command.linear.x = 0.0;
      command.angular.z = 0.8;
      return command;
    }

    if (std::isfinite(right_distance)) {
      const double error = wall_target_distance_ - right_distance;
      command.angular.z = clamp(error * wall_kp_, -0.8, 0.8);
    }

    return command;
  }

  bool is_safe_to_move(const geometry_msgs::msg::Twist & command) const
  {
    if (!got_scan_ || command.linear.x <= 0.0) {
      return true;
    }
    if (control_mode_ == "obstacle_avoidance" || control_mode_ == "wall_following") {
      return true;
    }
    return get_front_distance() >= obstacle_stop_distance_;
  }

  void emergency_stop()
  {
    current_cmd_ = geometry_msgs::msg::Twist();
    target_cmd_ = geometry_msgs::msg::Twist();
    publish_velocity(current_cmd_);
    RCLCPP_WARN_THROTTLE(get_logger(), *get_clock(), 1000, "unsafe front distance; emergency stop");
  }

  void limit_velocities(geometry_msgs::msg::Twist & command) const
  {
    command.linear.x = clamp(command.linear.x, -max_linear_speed_, max_linear_speed_);
    command.angular.z = clamp(command.angular.z, -max_angular_speed_, max_angular_speed_);
  }

  void smooth_velocities()
  {
    const double dt = 1.0 / control_rate_;
    current_cmd_.linear.x = make_simple_profile(
      current_cmd_.linear.x, target_cmd_.linear.x, max_linear_acceleration_ * dt);
    current_cmd_.angular.z = make_simple_profile(
      current_cmd_.angular.z, target_cmd_.angular.z, max_angular_acceleration_ * dt);
  }

  void publish_velocity(const geometry_msgs::msg::Twist & command)
  {
    geometry_msgs::msg::TwistStamped output;
    output.header.stamp = now();
    output.header.frame_id = "base_link";
    output.twist = command;
    cmd_pub_->publish(output);
  }

  double get_front_distance() const
  {
    return get_sector_min(-0.35, 0.35);
  }

  double get_right_distance() const
  {
    return get_sector_min(-1.75, -1.25);
  }

  double get_sector_min(double min_angle, double max_angle) const
  {
    if (!got_scan_) {
      return std::numeric_limits<double>::infinity();
    }

    double closest = std::numeric_limits<double>::infinity();
    for (size_t i = 0; i < last_scan_.ranges.size(); ++i) {
      const double angle = last_scan_.angle_min + static_cast<double>(i) *
        last_scan_.angle_increment;
      if (angle < min_angle || angle > max_angle) {
        continue;
      }
      const double range = last_scan_.ranges[i];
      if (std::isfinite(range)) {
        closest = std::min(closest, range);
      }
    }
    return closest;
  }

  static double make_simple_profile(double current, double target, double max_delta)
  {
    if (target > current) {
      return std::min(target, current + max_delta);
    }
    return std::max(target, current - max_delta);
  }

  static double clamp(double value, double lower, double upper)
  {
    return std::max(lower, std::min(value, upper));
  }

  void log_status()
  {
    if (!got_odom_) {
      RCLCPP_INFO(get_logger(), "waiting for /odom and optional /scan...");
      return;
    }

    const auto & p = last_odom_.pose.pose.position;
    const double front = get_front_distance();
    RCLCPP_INFO(
      get_logger(),
      "mode=%s x=%.3f y=%.3f front=%.2f joint_count=%zu",
      control_mode_.c_str(), p.x, p.y, front, last_joint_state_count_);
  }

  double max_linear_speed_;
  double max_angular_speed_;
  double max_linear_acceleration_;
  double max_angular_acceleration_;
  double command_timeout_;
  double obstacle_stop_distance_;
  double wall_target_distance_;
  double wall_kp_;
  double control_rate_;
  std::string control_mode_{"manual"};
  bool got_odom_{false};
  bool got_scan_{false};
  size_t last_joint_state_count_{0};
  rclcpp::Time last_cmd_time_{0, 0, RCL_ROS_TIME};
  geometry_msgs::msg::Twist last_teleop_;
  geometry_msgs::msg::Twist target_cmd_;
  geometry_msgs::msg::Twist current_cmd_;
  nav_msgs::msg::Odometry last_odom_;
  sensor_msgs::msg::LaserScan last_scan_;

  rclcpp::Publisher<geometry_msgs::msg::TwistStamped>::SharedPtr cmd_pub_;
  rclcpp::Subscription<geometry_msgs::msg::TwistStamped>::SharedPtr teleop_sub_;
  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr mode_sub_;
  rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr odom_sub_;
  rclcpp::Subscription<sensor_msgs::msg::JointState>::SharedPtr joint_state_sub_;
  rclcpp::Subscription<sensor_msgs::msg::LaserScan>::SharedPtr scan_sub_;
  rclcpp::Service<differential_drive_robot::srv::SetControlMode>::SharedPtr mode_srv_;
  OnSetParametersCallbackHandle::SharedPtr parameter_callback_handle_;
  rclcpp::TimerBase::SharedPtr control_timer_;
  rclcpp::TimerBase::SharedPtr status_timer_;
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<RobotController>());
  rclcpp::shutdown();
  return 0;
}
