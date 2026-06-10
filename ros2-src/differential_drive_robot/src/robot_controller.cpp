#include <algorithm>
#include <chrono>
#include <cmath>
#include <memory>

#include "geometry_msgs/msg/twist.hpp"
#include "geometry_msgs/msg/twist_stamped.hpp"
#include "nav_msgs/msg/odometry.hpp"
#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/joint_state.hpp"

using namespace std::chrono_literals;

class RobotController : public rclcpp::Node
{
public:
  RobotController()
  : Node("robot_controller")
  {
    max_linear_speed_ = declare_parameter("max_linear_speed", 0.5);
    max_angular_speed_ = declare_parameter("max_angular_speed", 1.2);
    command_timeout_ = declare_parameter("command_timeout", 0.7);

    cmd_pub_ = create_publisher<geometry_msgs::msg::TwistStamped>("cmd_vel", 10);

    teleop_sub_ = create_subscription<geometry_msgs::msg::Twist>(
      "teleop_cmd", 10,
      [this](const geometry_msgs::msg::Twist::SharedPtr msg) {
        last_cmd_ = *msg;
        last_cmd_time_ = now();
        publish_safe_command(*msg);
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

    watchdog_timer_ = create_wall_timer(100ms, [this]() {watchdog();});
    status_timer_ = create_wall_timer(2s, [this]() {log_status();});

    RCLCPP_INFO(
      get_logger(),
      "robot_controller ready: /teleop_cmd -> /cmd_vel, limits linear=%.2f angular=%.2f",
      max_linear_speed_, max_angular_speed_);
  }

private:
  void publish_safe_command(const geometry_msgs::msg::Twist & input)
  {
    geometry_msgs::msg::TwistStamped output;
    output.header.stamp = now();
    output.header.frame_id = "base_link";
    output.twist.linear.x = clamp(input.linear.x, -max_linear_speed_, max_linear_speed_);
    output.twist.angular.z = clamp(input.angular.z, -max_angular_speed_, max_angular_speed_);
    cmd_pub_->publish(output);
  }

  void publish_stop()
  {
    geometry_msgs::msg::Twist stop;
    publish_safe_command(stop);
  }

  void watchdog()
  {
    if (last_cmd_time_.nanoseconds() == 0) {
      return;
    }

    const double age = (now() - last_cmd_time_).seconds();
    const bool command_active =
      std::fabs(last_cmd_.linear.x) > 1e-4 || std::fabs(last_cmd_.angular.z) > 1e-4;

    if (command_active && age > command_timeout_) {
      publish_stop();
      last_cmd_ = geometry_msgs::msg::Twist();
      RCLCPP_WARN(get_logger(), "teleop command timed out; published stop command");
    }
  }

  void log_status()
  {
    if (!got_odom_) {
      RCLCPP_INFO(get_logger(), "waiting for /odom and /joint_states...");
      return;
    }

    const auto & p = last_odom_.pose.pose.position;
    RCLCPP_INFO(
      get_logger(),
      "state x=%.3f y=%.3f joint_count=%zu",
      p.x, p.y, last_joint_state_count_);
  }

  static double clamp(double value, double lower, double upper)
  {
    return std::max(lower, std::min(value, upper));
  }

  double max_linear_speed_;
  double max_angular_speed_;
  double command_timeout_;
  bool got_odom_{false};
  size_t last_joint_state_count_{0};
  rclcpp::Time last_cmd_time_{0, 0, RCL_ROS_TIME};
  geometry_msgs::msg::Twist last_cmd_;
  nav_msgs::msg::Odometry last_odom_;

  rclcpp::Publisher<geometry_msgs::msg::TwistStamped>::SharedPtr cmd_pub_;
  rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr teleop_sub_;
  rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr odom_sub_;
  rclcpp::Subscription<sensor_msgs::msg::JointState>::SharedPtr joint_state_sub_;
  rclcpp::TimerBase::SharedPtr watchdog_timer_;
  rclcpp::TimerBase::SharedPtr status_timer_;
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<RobotController>());
  rclcpp::shutdown();
  return 0;
}
