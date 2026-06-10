#include <termios.h>
#include <unistd.h>

#include <chrono>
#include <algorithm>
#include <iostream>
#include <memory>
#include <string>

#include "geometry_msgs/msg/twist.hpp"
#include "rclcpp/rclcpp.hpp"

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
    linear_step_ = declare_parameter("linear_step", 0.08);
    angular_step_ = declare_parameter("angular_step", 0.18);
    max_linear_ = declare_parameter("max_linear", 0.45);
    max_angular_ = declare_parameter("max_angular", 1.2);

    pub_ = create_publisher<geometry_msgs::msg::Twist>("teleop_cmd", 10);
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
      publish(0.0, 0.0);
      rclcpp::shutdown();
      return;
    }

    switch (c) {
      case 'i':
        linear_ += linear_step_;
        break;
      case ',':
        linear_ -= linear_step_;
        break;
      case 'j':
        angular_ += angular_step_;
        break;
      case 'l':
        angular_ -= angular_step_;
        break;
      case 'k':
      case ' ':
        linear_ = 0.0;
        angular_ = 0.0;
        break;
      default:
        return;
    }

    linear_ = clamp(linear_, -max_linear_, max_linear_);
    angular_ = clamp(angular_, -max_angular_, max_angular_);
    publish(linear_, angular_);
  }

  void publish(double linear, double angular)
  {
    geometry_msgs::msg::Twist msg;
    msg.linear.x = linear;
    msg.angular.z = angular;
    pub_->publish(msg);
    RCLCPP_INFO(get_logger(), "teleop_cmd linear=%.2f angular=%.2f", linear, angular);
  }

  static double clamp(double value, double lower, double upper)
  {
    return std::max(lower, std::min(value, upper));
  }

  void print_help() const
  {
    std::cout << "\nDifferential drive teleop\n"
              << "  i : increase forward speed\n"
              << "  , : increase reverse speed\n"
              << "  j : turn left\n"
              << "  l : turn right\n"
              << "  k/space : stop\n"
              << "  q : quit\n\n";
  }

  double linear_step_;
  double angular_step_;
  double max_linear_;
  double max_angular_;
  double linear_{0.0};
  double angular_{0.0};
  TerminalRawMode raw_mode_;
  rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr pub_;
  rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<TeleopKeyboard>());
  rclcpp::shutdown();
  return 0;
}
