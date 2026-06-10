#include "nav2_cpp_tutorial/nav2_client.hpp"

#include <chrono>
#include <cmath>
#include <future>
#include <string>

#include "tf2/LinearMath/Quaternion.h"
#include "tf2_geometry_msgs/tf2_geometry_msgs.hpp"

using namespace std::chrono_literals;

namespace nav2_cpp_tutorial
{

Nav2Client::Nav2Client(const rclcpp::NodeOptions & options)
: Node("nav2_client", options)
{
  nav_to_pose_client_ = rclcpp_action::create_client<NavigateToPose>(this, "navigate_to_pose");
  status_publisher_ = create_publisher<std_msgs::msg::String>("nav2_client/status", 10);

  odom_subscriber_ = create_subscription<nav_msgs::msg::Odometry>(
    "odom", 10, [this](const nav_msgs::msg::Odometry::SharedPtr msg) {
      odomCallback(msg);
    });

  laser_subscriber_ = create_subscription<sensor_msgs::msg::LaserScan>(
    "scan", 10, [this](const sensor_msgs::msg::LaserScan::SharedPtr msg) {
      laserCallback(msg);
    });

  publishStatus("idle");
}

bool Nav2Client::isServerReady(double timeout_sec)
{
  return nav_to_pose_client_->wait_for_action_server(
    std::chrono::duration_cast<std::chrono::nanoseconds>(
      std::chrono::duration<double>(timeout_sec)));
}

geometry_msgs::msg::PoseStamped Nav2Client::createPoseStamped(
  double x, double y, double yaw, const std::string & frame_id)
{
  geometry_msgs::msg::PoseStamped pose;
  pose.header.frame_id = frame_id;
  pose.header.stamp = now();
  pose.pose.position.x = x;
  pose.pose.position.y = y;
  pose.pose.position.z = 0.0;

  tf2::Quaternion q;
  q.setRPY(0.0, 0.0, yaw);
  pose.pose.orientation = tf2::toMsg(q);
  return pose;
}

bool Nav2Client::sendGoal(double x, double y, double yaw, const std::string & frame_id)
{
  return sendGoal(createPoseStamped(x, y, yaw, frame_id));
}

bool Nav2Client::sendGoal(const geometry_msgs::msg::PoseStamped & pose)
{
  if (!isServerReady(2.0)) {
    RCLCPP_ERROR(get_logger(), "navigate_to_pose action server is not ready");
    publishStatus("server_not_ready");
    return false;
  }

  NavigateToPose::Goal goal_msg;
  goal_msg.pose = pose;

  rclcpp_action::Client<NavigateToPose>::SendGoalOptions options;
  options.feedback_callback =
    [this](
    GoalHandleNav::SharedPtr goal_handle,
    const std::shared_ptr<const NavigateToPose::Feedback> feedback) {
      feedbackCallback(goal_handle, feedback);
    };
  options.result_callback =
    [this](const GoalHandleNav::WrappedResult & result) {
      resultCallback(result);
    };

  {
    std::lock_guard<std::mutex> lock(status_mutex_);
    goal_done_ = false;
    goal_success_ = false;
    navigation_status_ = "sending_goal";
  }
  publishStatus("sending_goal");

  auto future_goal_handle = nav_to_pose_client_->async_send_goal(goal_msg, options);
  if (future_goal_handle.wait_for(5s) != std::future_status::ready) {
    RCLCPP_ERROR(get_logger(), "timed out while sending navigation goal");
    publishStatus("goal_send_timeout");
    return false;
  }

  current_goal_handle_ = future_goal_handle.get();
  if (!current_goal_handle_) {
    RCLCPP_ERROR(get_logger(), "navigation goal was rejected");
    publishStatus("goal_rejected");
    return false;
  }

  RCLCPP_INFO(
    get_logger(), "goal accepted: x=%.2f y=%.2f",
    pose.pose.position.x, pose.pose.position.y);
  publishStatus("navigating");
  return true;
}

bool Nav2Client::waitForResult(double timeout_sec)
{
  std::unique_lock<std::mutex> lock(status_mutex_);
  const auto timeout = std::chrono::duration_cast<std::chrono::nanoseconds>(
    std::chrono::duration<double>(timeout_sec));
  const bool received = result_received_.wait_for(lock, timeout, [this]() {
        return goal_done_;
    });

  if (!received) {
    navigation_status_ = "timeout";
    publishStatus("timeout");
    return false;
  }

  return goal_success_;
}

bool Nav2Client::cancelGoal()
{
  if (!current_goal_handle_) {
    RCLCPP_WARN(get_logger(), "no active navigation goal to cancel");
    return false;
  }

  auto future_cancel = nav_to_pose_client_->async_cancel_goal(current_goal_handle_);
  if (future_cancel.wait_for(3s) != std::future_status::ready) {
    RCLCPP_ERROR(get_logger(), "timed out while cancelling navigation goal");
    return false;
  }

  publishStatus("cancel_requested");
  return true;
}

std::optional<geometry_msgs::msg::Pose> Nav2Client::getCurrentPose() const
{
  std::lock_guard<std::mutex> lock(status_mutex_);
  if (!has_pose_) {
    return std::nullopt;
  }
  return current_pose_;
}

std::string Nav2Client::getNavigationStatus() const
{
  std::lock_guard<std::mutex> lock(status_mutex_);
  return navigation_status_;
}

void Nav2Client::feedbackCallback(
  GoalHandleNav::SharedPtr,
  const std::shared_ptr<const NavigateToPose::Feedback> feedback)
{
  RCLCPP_INFO_THROTTLE(
    get_logger(), *get_clock(), 2000,
    "distance remaining: %.2f m, recoveries: %d",
    feedback->distance_remaining,
    feedback->number_of_recoveries);
}

void Nav2Client::resultCallback(const GoalHandleNav::WrappedResult & result)
{
  std::lock_guard<std::mutex> lock(status_mutex_);
  goal_done_ = true;

  switch (result.code) {
    case rclcpp_action::ResultCode::SUCCEEDED:
      goal_success_ = true;
      navigation_status_ = "succeeded";
      break;
    case rclcpp_action::ResultCode::ABORTED:
      goal_success_ = false;
      navigation_status_ = "aborted";
      break;
    case rclcpp_action::ResultCode::CANCELED:
      goal_success_ = false;
      navigation_status_ = "canceled";
      break;
    default:
      goal_success_ = false;
      navigation_status_ = "unknown_result";
      break;
  }

  publishStatus(navigation_status_);
  result_received_.notify_all();
}

void Nav2Client::odomCallback(const nav_msgs::msg::Odometry::SharedPtr msg)
{
  std::lock_guard<std::mutex> lock(status_mutex_);
  current_pose_ = msg->pose.pose;
  has_pose_ = true;
}

void Nav2Client::laserCallback(const sensor_msgs::msg::LaserScan::SharedPtr msg)
{
  std::lock_guard<std::mutex> lock(status_mutex_);
  current_laser_ = *msg;
}

void Nav2Client::publishStatus(const std::string & status)
{
  std_msgs::msg::String msg;
  msg.data = status;
  status_publisher_->publish(msg);
}

}  // namespace nav2_cpp_tutorial
