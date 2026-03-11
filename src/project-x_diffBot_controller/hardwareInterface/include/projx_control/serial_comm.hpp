#ifndef PROJX_CONTROL_SERIAL_COMM_HPP
#define PROJX_CONTROL_SERIAL_COMM_HPP

#include "protocol.hpp"
#include "rclcpp/rclcpp.hpp"
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <libserial/SerialPort.h>
#include <libserial/SerialStream.h>
#include <string>
#include <vector>

using namespace LibSerial;

class SerialBridge {
public:
  SerialBridge() : is_connected_(false), timeout_ms_(10) {}

  bool connect(const std::string &port_name, int baud_rate, int timeout_ms) {
    if (is_connected_)
      disconnect();

    try {
      serial_port_.Open(port_name);
      serial_port_.SetBaudRate(convertBaudRate(baud_rate));
      serial_port_.SetCharacterSize(CharacterSize::CHAR_SIZE_8);
      serial_port_.SetFlowControl(FlowControl::FLOW_CONTROL_NONE);
      serial_port_.SetParity(Parity::PARITY_NONE);
      serial_port_.SetStopBits(StopBits::STOP_BITS_1);

      this->timeout_ms_ = timeout_ms;

      // Note: LibSerial doesn't have a simple method to set a global read
      // timeout in ms the same way socket options do, but Read() calls can have
      // timeouts.

      is_connected_ = true;
      return true;
    } catch (...) {
      std::cerr << "Failed to open serial port: " << port_name << std::endl;
      is_connected_ = false;
      return false;
    }
  }

  void disconnect() {
    if (is_connected_) {
      try {
        serial_port_.Close();
      } catch (...) {
      }
      is_connected_ = false;
    }
  }

  bool connected() const { return is_connected_; }

  bool set_motor_values(float left_val, float right_val,
                        CommandType cmd = CMD_MOVE) {
    if (!is_connected_)
      return false;

    incomingPacket pkt;
    pkt.start = START_BYTE;
    pkt.seq = seq_counter_++;
    pkt.cmd = cmd;
    pkt.cmd_vel_left = left_val;
    pkt.cmd_vel_right = right_val;
    pkt.end = END_BYTE;

    try {
      DataBuffer buffer;
      const uint8_t *ptr = (const uint8_t *)&pkt;
      for (size_t i = 0; i < sizeof(pkt); ++i)
        buffer.push_back(ptr[i]);

      // if (++debug_print_counter_ > 50) {
      //   std::cout << "[SerialBridge] Sending CMD: " << (int)cmd
      //             << " | L: " << left_val << " | R: " << right_val <<
      //             std::endl;
      // }

      serial_port_.Write(buffer);
      // Removed DrainWriteBuffer to prevent blocking the ROS 2 loop
      return true;
    } catch (...) {
      return false;
    }
  }

  // bool read_encoder_values(int32_t &enc_l, int32_t &enc_r) {
  //   if (!is_connected_)
  //     return false;

  //   bool packet_read = false;

  //   try {
  //     size_t available = serial_port_.GetNumberOfBytesAvailable();
  //     if (available > 0) {
  //       DataBuffer raw_read;
  //       serial_port_.Read(raw_read, available, timeout_ms_);
  //       recv_buffer_.insert(recv_buffer_.end(), raw_read.begin(),
  //                           raw_read.end());
  //     }

  //     // Process all complete packets in the buffer
  //     while (recv_buffer_.size() >= sizeof(outgoingPacket)) {
  //       if (recv_buffer_[0] == START_BYTE) {
  //         outgoingPacket pkt;
  //         std::memcpy(&pkt, recv_buffer_.data(), sizeof(outgoingPacket));

  //         if (pkt.end == END_BYTE) {
  //           // Valid packet found
  //           enc_l = pkt.enc_count_left;
  //           enc_r = pkt.enc_count_right;
  //           packet_read = true;

  //           if (!first_packet_received_) {
  //             std::cout
  //                 << "[SerialBridge] First packet received! Start byte: 0x"
  //                 << std::hex << (int)pkt.start << std::dec << std::endl;
  //             first_packet_received_ = true;
  //           }

  //           // Erase the processed packet from buffer
  //           recv_buffer_.erase(recv_buffer_.begin(),
  //                              recv_buffer_.begin() + sizeof(outgoingPacket));
  //         } else {
  //           // Start byte matched, but end byte mismatched. Corrupted packet.
  //           recv_buffer_.erase(recv_buffer_.begin());
  //         }
  //       } else {
  //         // Not a start byte. Shift buffer by 1 to search for the next start
  //         // byte.
  //         recv_buffer_.erase(recv_buffer_.begin());
  //       }
  //     }

  //     // Cap buffer size to prevent memory leaks if completely desynced
  //     if (recv_buffer_.size() > 1024) {
  //       recv_buffer_.clear();
  //     }

  //     if (packet_read) {
  //       if (++debug_print_counter_ > 50) {
  //         std::cout << "[SerialBridge] Encoders -> L: " << enc_l
  //                   << " | R: " << enc_r << std::endl;
  //         debug_print_counter_ = 0;
  //       }
  //       return true;
  //     }
  //     return false;

  //   } catch (...) {
  //     return false;
  //   }
  // }

  bool read_encoder_values(int32_t &enc_l, int32_t &enc_r) {
  if (!is_connected_) return false;

  bool packet_read = false;
  try {
    size_t available = serial_port_.GetNumberOfBytesAvailable();
    if (available > 0) {
      DataBuffer raw_read;
      serial_port_.Read(raw_read, available, timeout_ms_);
      recv_buffer_.insert(recv_buffer_.end(), raw_read.begin(), raw_read.end());
    }

    // New packet size is 55 bytes (1+4+1+40+4+4+1)
    while (recv_buffer_.size() >= sizeof(outgoingPacket)) {
      if (recv_buffer_[0] == START_BYTE) {
        outgoingPacket pkt;
        std::memcpy(&pkt, recv_buffer_.data(), sizeof(outgoingPacket));

        if (pkt.end == END_BYTE) {
          // Valid packet found
          enc_l = pkt.enc_count_left;
          enc_r = pkt.enc_count_right;
          
          // Copy IMU data to the class member address
          std::memcpy(&latest_imu_data_, &pkt.imu_vals, sizeof(imuData));
          
          packet_read = true;
          recv_buffer_.erase(recv_buffer_.begin(), recv_buffer_.begin() + sizeof(outgoingPacket));
        } else {
          recv_buffer_.erase(recv_buffer_.begin());
        }
      } else {
        recv_buffer_.erase(recv_buffer_.begin());
      }
    }
    // ... Cap buffer logic ...
    return packet_read;
  } catch (...) { return false; }
}

  // This provides the pointer (address) to the latest data
const imuData* get_latest_imu_ptr() const {
    return &latest_imu_data_;
}

private:
  imuData latest_imu_data_;
  SerialPort serial_port_;
  bool is_connected_;
  DataBuffer recv_buffer_;

  int timeout_ms_;
  uint32_t seq_counter_ = 0;
  bool first_packet_received_ = false;
  int debug_print_counter_ = 0;

  BaudRate convertBaudRate(int baud) {
    switch (baud) {
    case 1200:
      return BaudRate::BAUD_1200;
    case 1800:
      return BaudRate::BAUD_1800;
    case 2400:
      return BaudRate::BAUD_2400;
    case 4800:
      return BaudRate::BAUD_4800;
    case 9600:
      return BaudRate::BAUD_9600;
    case 19200:
      return BaudRate::BAUD_19200;
    case 38400:
      return BaudRate::BAUD_38400;
    case 57600:
      return BaudRate::BAUD_57600;
    case 115200:
      return BaudRate::BAUD_115200;
    case 230400:
      return BaudRate::BAUD_230400;
    default:
      return BaudRate::BAUD_115200;
    }
  }
};

#endif // PROJX_CONTROL_SERIAL_COMM_HPP
