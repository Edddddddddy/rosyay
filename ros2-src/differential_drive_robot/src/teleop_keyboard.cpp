#include <termios.h>
#include <unistd.h>

#include <algorithm>
#include <chrono>
#include <iostream>
#include <memory>
#include <string>

#include "geometry_msgs/msg/twist_stamped.hpp"
#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

using namespace std::chrono_literals;

class TerminalRawMode
{
public:
  TerminalRawMode()
  {
    tcgetattr(STDIN_FILENO, &original_);
    termios raw = original_;
    raw.c_lflag &= static_cast<unsigned int>(~(ICANON | ECHO));
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
  }

  ~TerminalRawMode()
  {
    tcsetattr(STDIN_FILENO, TCSANOW, &original_);
  }

private:
  termios original_{};
};

class TeleopKeyboard : public rclcpp::Node
{
public:
  TeleopKeyboard()
  : Node("teleop_keyboard")
  {
    linear_step_ = declare_parameter("linear_step", 0.04);
    angular_step_ = declare_parameter("angular_step", 0.2);
    max_linear_ = declare_parameter("max_linear", 0.26);
    max_angular_ = declare_parameter("max_angular", 1.82);

    twist_pub_ = create_publisher<geometry_msgs::msg::TwistStamped>("teleop_cmd", 10);
    mode_pub_ = create_publisher<std_msgs::msg::String>("control_mode", 10);
    timer_ = create_wall_timer(50ms, [this]() {poll_key();});
    print_help();
  }

private:
  void poll_key()
  {
    char c = 0;
    const ssize_t n = read(STDIN_FILENO, &c, 1);
    if (n <= 0) {
      return;
    }

    if (c == 'q') {
      publish_twist(0.0, 0.0);
      rclcpp::shutdown();
      return;
    }

    if (process_mode_key(c)) {
      return;
    }

    if (!process_motion_key(c)) {
      return;
    }

    linear_ = clamp(linear_, -max_linear_, max_linear_);
    angular_ = clamp(angular_, -max_angular_, max_angular_);
    publish_twist(linear_, angular_);
  }

  bool process_motion_key(char c)
  {
    switch (c) {
      case 'w':
        linear_ += linear_step_;
        break;
      case 'x':
        linear_ -= linear_step_;
        break;
      case 'a':
        angular_ += angular_step_;
        break;
      case 'd':
        angular_ -= angular_step_;
        break;
      case 's':
      case ' ':
        linear_ = 0.0;
        angular_ = 0.0;
        break;
      default:
        return false;
    }
    return true;
  }

  bool process_mode_key(char c)
  {
    if (c == '1') {
      publish_mode("manual");
    } else if (c == '2') {
      publish_mode("auto_forward");
    } else if (c == '3') {
      publish_mode("auto_circle");
    } else if (c == '4') {
      publish_mode("obstacle_avoidance");
    } else if (c == '0') {
      publish_mode("wall_following");
    } else {
      return false;
    }

    linear_ = 0.0;
    angular_ = 0.0;
    publish_twist(0.0, 0.0);
    return true;
  }

  void publish_twist(double linear, double angular)
  {
    geometry_msgs::msg::TwistStamped msg;
    msg.header.stamp = now();
    msg.header.frame_id = "base_link";
    msg.twist.linear.x = linear;
    msg.twist.angular.z = angular;
    twist_pub_->publish(msg);
    RCLCPP_INFO(get_logger(), "teleop_cmd linear=%.2f angular=%.2f", linear, angular);
  }

  void publish_mode(const std::string & mode)
  {
    std_msgs::msg::String msg;
    msg.data = mode;
    mode_pub_->publish(msg);
    RCLCPP_INFO(get_logger(), "control_mode=%s", mode.c_str());
  }

  static double clamp(double value, double lower, double upper)
  {
    return std::max(lower, std::min(value, upper));
  }

  void print_help() const
  {
    std::cout << "\nDifferential drive teleop\n"
              << "  w/x : increase forward/reverse speed\n"
              << "  a/d : turn left/right\n"
              << "  s/space : stop\n"
              << "  1 : manual\n"
              << "  2 : auto_forward\n"
              << "  3 : auto_circle\n"
              << "  4 : obstacle_avoidance\n"
              << "  0 : wall_following\n"
              << "  q : quit\n\n";
  }

  double linear_step_;
  double angular_step_;
  double max_linear_;
  double max_angular_;
  double linear_{0.0};
  double angular_{0.0};
  TerminalRawMode raw_mode_;
  rclcpp::Publisher<geometry_msgs::msg::TwistStamped>::SharedPtr twist_pub_;
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr mode_pub_;
  rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<TeleopKeyboard>());
  rclcpp::shutdown();
  return 0;
}
