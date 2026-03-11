#ifndef PROJX_CONTROL_UDP_COMM_HPP
#define PROJX_CONTROL_UDP_COMM_HPP

#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "rclcpp/rclcpp.hpp"

#include "protocol.hpp" // Ensure this file is in your include path
#define PACKET_START_BYTE 0xA5
class UdpBridge {
public:
    UdpBridge() : sockfd_(-1) {}

    bool connect(const std::string& ip, int port, int timeout_ms) {
        if (sockfd_ != -1) disconnect();

        sockfd_ = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd_ < 0) {
            std::cerr << "Socket creation failed" << std::endl;
            return false;
        }

        memset(&servaddr_, 0, sizeof(servaddr_));
        servaddr_.sin_family = AF_INET;
        servaddr_.sin_port = htons(port);
        if (inet_pton(AF_INET, ip.c_str(), &servaddr_.sin_addr) <= 0) {
            std::cerr << "Invalid IP address" << std::endl;
            return false;
        }

        struct timeval tv;
        tv.tv_sec = timeout_ms / 1000;
        tv.tv_usec = (timeout_ms % 1000) * 1000;

        setsockopt(sockfd_, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

        if (::connect(sockfd_, (struct sockaddr *)&servaddr_, sizeof(servaddr_)) < 0) {
            std::cerr << "Connection failed" << std::endl;
            return false;
        }

        is_connected_ = true;
        return true;
    }

    void disconnect() {
        if (sockfd_ != -1) {
            close(sockfd_);
            sockfd_ = -1;
        }
        is_connected_ = false;
    }

    bool connected() const {
        return is_connected_;
    }

    // --- NEW: Send Velocity Command ---
    bool set_motor_values(float left_val, float right_val, CommandType cmd = CMD_MOVE) {
        if (!is_connected_) return false;

        incomingPacket pkt;
        pkt.magic = PACKET_START_BYTE; // 0xA5
        pkt.seq = seq_counter_++;
        pkt.cmd = cmd;
        pkt.cmd_vel_left = left_val;
        pkt.cmd_vel_right = right_val;
        pkt.crc32 = 0; // Placeholder
    RCLCPP_INFO(rclcpp::get_logger("ProjXDriveHW"), "rpm -> L: %f | R: %f", left_val, right_val);

        // Calculate CRC (excluding the CRC field itself, which is last 4 bytes)
        pkt.crc32 = calculateCRC32((const uint8_t*)&pkt, INCOMING_PACKET_SIZE - 4);

        ssize_t sent_bytes = send(sockfd_, &pkt, sizeof(pkt), 0);
        return (sent_bytes == sizeof(pkt));
    }

    // --- NEW: Read Encoder Feedback ---
    bool read_encoder_values(int32_t &enc_l, int32_t &enc_r) {
        if (!is_connected_) return false;

        outgoingPacket pkt;
        ssize_t len = recv(sockfd_, &pkt, sizeof(pkt), MSG_DONTWAIT);
        // Basic Validation
        if (len == sizeof(outgoingPacket) && pkt.magic == PACKET_START_BYTE) {
            // Optional: Verify CRC here if you want extra safety
            
            enc_l = pkt.enc_count_left;
            enc_r = pkt.enc_count_right;
            return true;
        }
        return false;
    }

private:
    int sockfd_;
    struct sockaddr_in servaddr_;
    bool is_connected_ = false;
    uint32_t seq_counter_ = 0;

    // --- CRC32 Implementation (Standard Ethernet Polynomial) ---
    uint32_t calculateCRC32(const uint8_t *data, size_t length) {
        uint32_t crc = 0xFFFFFFFF;
        for (size_t i = 0; i < length; i++) {
            crc ^= (uint32_t)data[i] << 24;
            for (int j = 0; j < 8; j++) {
                if (crc & 0x80000000) crc = (crc << 1) ^ 0x04C11DB7;
                else crc <<= 1;
            }
        }
        return crc;
    }
};

#endif // PROJX_CONTROL_UDP_COMM_HPP