// Copyright 2021 ros2_control Development Team
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef PROKECTX_HW__DIFFBOT_SYSTEM_HPP_
#define PROKECTX_HW__DIFFBOT_SYSTEM_HPP_

#include <memory>
#include <string>
#include <vector>

#include "hardware_interface/handle.hpp"
#include "hardware_interface/hardware_info.hpp"
#include "hardware_interface/system_interface.hpp"
#include "hardware_interface/types/hardware_interface_return_values.hpp"
#include "projx_control/visibility_control.h"
#include "sensor_msgs/msg/imu.hpp" 
#include "rclcpp/rclcpp.hpp"
#include "rclcpp/clock.hpp"
#include "rclcpp/duration.hpp"
#include "rclcpp/macros.hpp"
#include "rclcpp/time.hpp"
#include "rclcpp_lifecycle/node_interfaces/lifecycle_node_interface.hpp"
#include "rclcpp_lifecycle/state.hpp"

#include "projx_control/serial_comm.hpp"
#include "projx_control/wheel.hpp"

namespace projx_control {
class ProjXDriveHW : public hardware_interface::SystemInterface {

  struct Config {
    std::string left_wheel_name = "";
    std::string right_wheel_name = "";
    float loop_rate = 0.0;
    std::string device = "";
    int baud_rate = 0;
    int timeout_ms = 0;
    int enc_counts_per_rev = 0;
    int pid_p = 0;
    int pid_d = 0;
    int pid_i = 0;
    int pid_o = 0;
  };

public:
  RCLCPP_SHARED_PTR_DEFINITIONS(ProjXDriveHW);

  PROKECTX_HW_PUBLIC
  hardware_interface::CallbackReturn
  on_init(const hardware_interface::HardwareInfo &info) override;

  PROKECTX_HW_PUBLIC
  std::vector<hardware_interface::StateInterface>
  export_state_interfaces() override;

  PROKECTX_HW_PUBLIC
  std::vector<hardware_interface::CommandInterface>
  export_command_interfaces() override;

  PROKECTX_HW_PUBLIC
  hardware_interface::CallbackReturn
  on_configure(const rclcpp_lifecycle::State &previous_state) override;

  PROKECTX_HW_PUBLIC
  hardware_interface::CallbackReturn
  on_cleanup(const rclcpp_lifecycle::State &previous_state) override;

  PROKECTX_HW_PUBLIC
  hardware_interface::CallbackReturn
  on_activate(const rclcpp_lifecycle::State &previous_state) override;

  PROKECTX_HW_PUBLIC
  hardware_interface::CallbackReturn
  on_deactivate(const rclcpp_lifecycle::State &previous_state) override;

  PROKECTX_HW_PUBLIC
  hardware_interface::return_type read(const rclcpp::Time &time,
                                       const rclcpp::Duration &period) override;

  PROKECTX_HW_PUBLIC
  hardware_interface::return_type
  write(const rclcpp::Time &time, const rclcpp::Duration &period) override;

private:
  SerialBridge comms_;
  Config cfg_;
  Wheel wheel_l_;
  Wheel wheel_r_;
  rclcpp::Publisher<sensor_msgs::msg::Imu>::SharedPtr imu_pub_;
  rclcpp::Node::SharedPtr imu_node_;
};

} // namespace projx_control

#endif // PROKECTX_HW__DIFFBOT_SYSTEM_HPP_
