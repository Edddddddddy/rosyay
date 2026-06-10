#include <chrono>
#include <cmath>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "nav2_cpp_tutorial/nav2_client.hpp"
#include "rclcpp/rclcpp.hpp"

using namespace nav2_cpp_tutorial;

struct TestPoint
{
  double x;
  double y;
  double yaw;
  std::string description;
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);

  auto nav2_client = std::make_shared<Nav2Client>();
  std::thread spin_thread([nav2_client]() {
      rclcpp::spin(nav2_client);
    });

  std::cout << "Navigation2 C++ basic test" << std::endl;
  std::cout << "Waiting for navigate_to_pose action server..." << std::endl;

  if (!nav2_client->isServerReady(10.0)) {
    std::cout << "Navigation2 is not ready. Start Nav2 before running this test." << std::endl;
    rclcpp::shutdown();
    spin_thread.join();
    return 1;
  }

  const std::vector<TestPoint> test_points = {
    {1.0, 0.0, 0.0, "forward 1 meter"},
    {1.0, 1.0, 1.57, "front-right, face north"},
    {0.0, 1.0, 3.14, "left side, face west"},
    {0.0, 0.0, 0.0, "return to origin"},
  };

  int success_count = 0;
  for (size_t i = 0; i < test_points.size(); ++i) {
    const auto & point = test_points[i];
    std::cout << "\nTest " << (i + 1) << "/" << test_points.size()
              << ": " << point.description << std::endl;
    std::cout << "Goal: x=" << point.x << " y=" << point.y
              << " yaw=" << point.yaw * 180.0 / M_PI << " deg" << std::endl;

    if (!nav2_client->sendGoal(point.x, point.y, point.yaw)) {
      std::cout << "Failed to send goal" << std::endl;
      continue;
    }

    if (nav2_client->waitForResult(60.0)) {
      ++success_count;
      std::cout << "Navigation succeeded" << std::endl;
    } else {
      std::cout << "Navigation failed or timed out. status="
                << nav2_client->getNavigationStatus() << std::endl;
    }

    std::this_thread::sleep_for(std::chrono::seconds(2));
  }

  std::cout << "\nSummary: " << success_count << "/" << test_points.size()
            << " goals succeeded" << std::endl;

  rclcpp::shutdown();
  spin_thread.join();
  return success_count == static_cast<int>(test_points.size()) ? 0 : 2;
}
