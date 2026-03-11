#pragma once
#include <Arduino.h>
#include "packet.hpp" 

class SerialManager {
    private:
        uint8_t incoming_buffer[INCOMING_PACKET_SIZE];
        uint8_t outgoing_buffer[OUTGOING_PACKET_SIZE];

    public:
        int read() {
            while (Serial.available() >= INCOMING_PACKET_SIZE) {
                if (Serial.peek() != START_BYTE) {
                    Serial.read(); 
                    continue;
                }

                Serial.readBytes(incoming_buffer, INCOMING_PACKET_SIZE);
                
                incomingPacket* pkt = (incomingPacket*)incoming_buffer;
                if (pkt->end == END_BYTE) {
                    return INCOMING_PACKET_SIZE; 
                }
            }
            return 0;
        }

        // Standard sendReply for the manager
        void sendReply() {
            Serial.write(outgoing_buffer, OUTGOING_PACKET_SIZE);
        }

        incomingPacket* getIncomingPacket() {
            return (incomingPacket*)incoming_buffer;
        }

        outgoingPacket* getOutgoingPacket() {
            return (outgoingPacket*)outgoing_buffer;
        }
};