#pragma once 
#include "imu.hpp"
#include <stdint.h>

#define START_BYTE 0xA5
#define END_BYTE   0x5A

#pragma pack(push, 1)
struct incomingPacket {
    uint8_t start;       // 0xA5
    uint32_t seq;        
    uint8_t cmd;        
    float cmd_vel_left;          
    float cmd_vel_right; 
    uint8_t end;         // 0x5A
};

struct outgoingPacket {
    uint8_t start;       // 0xA5
    uint32_t seq;  
    uint8_t status;  
    imuPacket imu_data;     
    int32_t enc_count_left;          
    int32_t enc_count_right; 
    uint8_t end;         // 0x5A
};
#pragma pack(pop)

const size_t INCOMING_PACKET_SIZE = sizeof(incomingPacket); 
const size_t OUTGOING_PACKET_SIZE = sizeof(outgoingPacket);