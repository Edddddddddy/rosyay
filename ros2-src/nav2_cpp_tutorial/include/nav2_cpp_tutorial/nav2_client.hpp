#ifndef NAV2_CPP_TUTORIAL__NAV2_CLIENT_HPP_
#define NAV2_CPP_TUTORIAL__NAV2_CLIENT_HPP_

#include <condition_variable>
#include <memory>
#include <mutex>
#include <optional>
#include <string>

#include "geometry_msgs/msg/pose.hpp"
#include "geometry_msgs/msg/pose_stamped.hpp"
#include "nav2_msgs/action/navigate_to_pose.hpp"
#include "nav_msgs/msg/odometry.hpp"
#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "sensor_msgs/msg/laser_scan.hpp"
#include "std_msgs/msg/string.hpp"

namespace nav2_cpp_tutorial
{

class Nav2Client : public rclcpp::Node
{
public:
  using NavigateToPose = nav2_msgs::action::NavigateToPose;
  using GoalHandleNav = rclcpp_action::ClientGoalHandle<NavigateToPose>;

  explicit Nav2Client(const rclcpp::NodeOptions & options = rclcpp::NodeOptions());
  ~Nav2Client() override = default;

  bool sendGoal(double x, double y, double yaw = 0.0, const std::string & frame_id = "map");
  bool sendGoal(const geometry_msgs::msg::PoseStamped & pose);
  bool waitForResult(double timeout_sec = 60.0);
  bool cancelGoal();
  bool isServerReady(double timeout_sec = 5.0);
  std::optional<geometry_msgs::msg::Pose> getCurrentPose() const;
  std::string getNavigationStatus() const;
  geometry_msgs::msg::PoseStamped createPoseStamped(
    double x, double y, double yaw, const std::string & frame_id = "map");

private:
  void feedbackCallback(
    GoalHandleNav::SharedPtr,
    const std::shared_ptr<const NavigateToPose::Feedback> feedback);
  void resultCallback(const GoalHandleNav::WrappedResult & result);
  void odomCallback(const nav_msgs::msg::Odometry::SharedPtr msg);
  void laserCallback(const sensor_msgs::msg::LaserScan::SharedPtr msg);
  void publishStatus(const std::string & status);

  rclcpp_action::Client<NavigateToPose>::SharedPtr nav_to_pose_client_;
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr status_publisher_;
  rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr odom_subscriber_;
  rclcpp::Subscription<sensor_msgs::msg::LaserScan>::SharedPtr laser_subscriber_;

  GoalHandleNav::SharedPtr current_goal_handle_;
  geometry_msgs::msg::Pose current_pose_;
  sensor_msgs::msg::LaserScan current_laser_;
  std::string navigation_status_{"idle"};
  bool has_pose_{false};
  bool goal_done_{false};
  bool goal_success_{false};

  mutable std::mutex status_mutex_;
  std::condition_variable result_received_;
};

}  // namespace nav2_cpp_tutorial

#endif  // NAV2_CPP_TUTORIAL__NAV2_CLIENT_HPP_
