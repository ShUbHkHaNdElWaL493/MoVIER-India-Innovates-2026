#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stddef.h>
#include <stdint.h>
#define START_BYTE 0xA5
#define END_BYTE 0x5A
enum CommandType : uint8_t {
  CMD_STOP = 0x00,
  CMD_MOVE = 0x01,
  CMD_RESET_VARIABLE = 0x02,
  CMD_RESET_CONTROLLER = 0x03,
  CMD_OPEN_LOOP_CONTROL = 0x04,
  EMG_STOP = 0x05
};

// FOR FURTHER IMPLEMENTATION
enum RobotStatus : uint8_t {
  STATUS_OK = 0x00,
  STATUS_ERROR_BATTERY = 0x01,
  STATUS_ERROR_CRC = 0x02,
  STATUS_ESTOP = 0x03
};
struct imuData {
    float qx, qy, qz, qw;
    float ax, ay, az;
    float gx, gy, gz;
};

#pragma pack(push, 1)
struct incomingPacket {
  uint8_t start; // 0xA5
  uint32_t seq;
  uint8_t cmd;
  float cmd_vel_left;
  float cmd_vel_right;
  uint8_t end; // 0x5A
};

struct outgoingPacket {
    uint8_t start;       
    uint32_t seq;  
    uint8_t status;  
    imuData imu_vals;    
    int32_t enc_count_left;          
    int32_t enc_count_right; 
    uint8_t end;        
};
#pragma pack(pop)

const size_t INCOMING_PACKET_SIZE = sizeof(incomingPacket);
const size_t OUTGOING_PACKET_SIZE = sizeof(outgoingPacket);

#endif
