#include "projx_control/projx-diffdrive.hpp"
#include "hardware_interface/types/hardware_interface_type_values.hpp"

#include <chrono>
#include <cmath>
#include <limits>
#include <memory>
#include <vector>

namespace projx_control {

// ... (on_init stays mostly the same, ensuring config loading is robust) ...

hardware_interface::CallbackReturn
ProjXDriveHW::on_init(const hardware_interface::HardwareInfo &info) {
  if (hardware_interface::SystemInterface::on_init(info) !=
      hardware_interface::CallbackReturn::SUCCESS) {
    return hardware_interface::CallbackReturn::ERROR;
  }

  // Load Parameters
  cfg_.left_wheel_name = info_.hardware_parameters["left_wheel_name"];
  cfg_.right_wheel_name = info_.hardware_parameters["right_wheel_name"];
  cfg_.loop_rate = std::stof(info_.hardware_parameters["loop_rate"]);
  cfg_.device = info_.hardware_parameters["device"];
  cfg_.baud_rate = std::stoi(
      info_.hardware_parameters["baud_rate"]); // Actually Port Number for UDP
  cfg_.timeout_ms = std::stoi(info_.hardware_parameters["timeout_ms"]);
  cfg_.enc_counts_per_rev =
      std::stoi(info_.hardware_parameters["enc_counts_per_rev"]);

  // Set up Wheels
  wheel_l_.setup(cfg_.left_wheel_name, cfg_.enc_counts_per_rev);
  wheel_r_.setup(cfg_.right_wheel_name, cfg_.enc_counts_per_rev);
  
  return hardware_interface::CallbackReturn::SUCCESS;
}

// ... (export_state_interfaces and export_command_interfaces stay the same) ...
std::vector<hardware_interface::StateInterface>
ProjXDriveHW::export_state_interfaces() {
  std::vector<hardware_interface::StateInterface> state_interfaces;
  state_interfaces.emplace_back(hardware_interface::StateInterface(
      wheel_l_.name, hardware_interface::HW_IF_POSITION, &wheel_l_.pos));
  state_interfaces.emplace_back(hardware_interface::StateInterface(
      wheel_l_.name, hardware_interface::HW_IF_VELOCITY, &wheel_l_.vel));
  state_interfaces.emplace_back(hardware_interface::StateInterface(
      wheel_r_.name, hardware_interface::HW_IF_POSITION, &wheel_r_.pos));
  state_interfaces.emplace_back(hardware_interface::StateInterface(
      wheel_r_.name, hardware_interface::HW_IF_VELOCITY, &wheel_r_.vel));
  return state_interfaces;
}

std::vector<hardware_interface::CommandInterface>
ProjXDriveHW::export_command_interfaces() {
  std::vector<hardware_interface::CommandInterface> command_interfaces;
  command_interfaces.emplace_back(hardware_interface::CommandInterface(
      wheel_l_.name, hardware_interface::HW_IF_VELOCITY, &wheel_l_.cmd));
  command_interfaces.emplace_back(hardware_interface::CommandInterface(
      wheel_r_.name, hardware_interface::HW_IF_VELOCITY, &wheel_r_.cmd));
  return command_interfaces;
}

hardware_interface::CallbackReturn
ProjXDriveHW::on_configure(const rclcpp_lifecycle::State & /*previous_state*/) {
  RCLCPP_INFO(rclcpp::get_logger("ProjXDriveHW"), "Configuring Serial...");
  
  if (comms_.connected())
    comms_.disconnect();

  if (comms_.connect(cfg_.device, cfg_.baud_rate, cfg_.timeout_ms)) {
    // Check if node already exists before recreating to prevent memory leaks/crashes
    if (!imu_node_) {
        imu_node_ = std::make_shared<rclcpp::Node>("imu_sensor_node");
        imu_pub_ = imu_node_->create_publisher<sensor_msgs::msg::Imu>("/imu/data_raw", 10);
    }
    return hardware_interface::CallbackReturn::SUCCESS;
  } else {
    return hardware_interface::CallbackReturn::ERROR;
  }
}
// ... (on_cleanup, on_activate, on_deactivate stay standard) ...
hardware_interface::CallbackReturn
ProjXDriveHW::on_cleanup(const rclcpp_lifecycle::State &) {
  comms_.disconnect();
  return hardware_interface::CallbackReturn::SUCCESS;
}
hardware_interface::CallbackReturn
ProjXDriveHW::on_activate(const rclcpp_lifecycle::State &) {
  return hardware_interface::CallbackReturn::SUCCESS;
}
hardware_interface::CallbackReturn
ProjXDriveHW::on_deactivate(const rclcpp_lifecycle::State &) {
  // Release publisher to prevent read() from trying to publish while inactive
  imu_pub_.reset(); 
  return hardware_interface::CallbackReturn::SUCCESS;
}

// --- READ FUNCTION ---
hardware_interface::return_type
ProjXDriveHW::read(const rclcpp::Time &time, const rclcpp::Duration &period) {
  if (!comms_.connected())
    return hardware_interface::return_type::ERROR;

  int32_t raw_l = 0;
  int32_t raw_r = 0;

  if (comms_.read_encoder_values(raw_l, raw_r)) {
    wheel_l_.enc = raw_l;
    wheel_r_.enc = raw_r;

    // Safety check: Only publish if the shared pointer is valid
    if (imu_pub_) {
      const imuData* raw_imu = comms_.get_latest_imu_ptr();
      auto imu_msg = sensor_msgs::msg::Imu();

      imu_msg.header.stamp = time;
      imu_msg.header.frame_id = "imu_link";


      // Orientation
      imu_msg.orientation.x = raw_imu->qx;
      imu_msg.orientation.y = raw_imu->qy;
      imu_msg.orientation.z = raw_imu->qz;
      imu_msg.orientation.w = raw_imu->qw;

      // Linear Acceleration
      imu_msg.linear_acceleration.x = raw_imu->ax;
      imu_msg.linear_acceleration.y = raw_imu->ay;
      imu_msg.linear_acceleration.z = raw_imu->az;

      // Angular Velocity
      imu_msg.angular_velocity.x = raw_imu->gx;
      imu_msg.angular_velocity.y = raw_imu->gy;
      imu_msg.angular_velocity.z = raw_imu->gz;

      imu_msg.orientation_covariance[0] = 0.001; // Roll variance
      imu_msg.orientation_covariance[4] = 0.001; // Pitch variance
      imu_msg.orientation_covariance[8] = 0.001; // Yaw variance

      // Angular Velocity Covariance
      imu_msg.angular_velocity_covariance[0] = 0.01;
      imu_msg.angular_velocity_covariance[4] = 0.01;
      imu_msg.angular_velocity_covariance[8] = 0.01;

      // Linear Acceleration Covariance (Slightly noisier)
      imu_msg.linear_acceleration_covariance[0] = 0.05;
      imu_msg.linear_acceleration_covariance[4] = 0.05;
      imu_msg.linear_acceleration_covariance[8] = 0.05;

      imu_pub_->publish(imu_msg);
    }
  }

  // Wheel position/velocity math
  double delta_seconds = period.seconds();
  double pos_prev = wheel_l_.pos;
  wheel_l_.pos = wheel_l_.calc_enc_angle(); 
  wheel_l_.vel = (wheel_l_.pos - pos_prev) / delta_seconds;

  pos_prev = wheel_r_.pos;
  wheel_r_.pos = wheel_r_.calc_enc_angle();
  wheel_r_.vel = (wheel_r_.pos - pos_prev) / delta_seconds;

  return hardware_interface::return_type::OK;
}
// --- WRITE FUNCTION ---
hardware_interface::return_type
projx_control::ProjXDriveHW::write(const rclcpp::Time & /*time*/,
                                   const rclcpp::Duration & /*period*/) {
  if (!comms_.connected())
    return hardware_interface::return_type::ERROR;

  float left_rpm = (wheel_l_.cmd * 60.0) / (2 * M_PI);
  float right_rpm = (wheel_r_.cmd * 60.0) / (2 * M_PI);

  bool success = comms_.set_motor_values(left_rpm, right_rpm, CMD_MOVE);

  RCLCPP_INFO(rclcpp::get_logger("DiffBotHardware"), "LEFT_RPM: %.2f, RIGHT_RPM: %.2f", left_rpm, right_rpm);

  
  if (!success) {
    RCLCPP_ERROR(rclcpp::get_logger("ProjXDriveHW"),
                 // *rclcpp::get_clock(),
                 // 1000, // Log at most once every 1000ms
                 "Failed to send Serial packet! Check connection.");
  }

  return hardware_interface::return_type::OK;
}

} // namespace projx_control

#include "pluginlib/class_list_macros.hpp"
PLUGINLIB_EXPORT_CLASS(projx_control::ProjXDriveHW,
                       hardware_interface::SystemInterface)