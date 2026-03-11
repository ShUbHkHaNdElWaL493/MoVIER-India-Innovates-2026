#pragma once

#include <Arduino.h>
#include <LwIP.h>
#include <STM32Ethernet.h>
#include <EthernetUdp.h>
#include "packet.hpp" 

class NetworkManager {
private:
    EthernetUDP udp;
    
    uint8_t incoming_buffer[INCOMING_PACKET_SIZE]; 
    uint8_t outgoing_buffer[OUTGOING_PACKET_SIZE]; 

    IPAddress lastSenderIP;
    uint16_t lastSenderPort;
    bool isConnected = false;

    byte mac[6] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
    IPAddress ip;
    IPAddress dns;
    IPAddress gateway;
    IPAddress subnet;
    unsigned int localPort;

public:
    
    NetworkManager() 
    {
        // Default Config
        ip = IPAddress(192,168,0,15);
        dns = IPAddress(0, 0, 0, 0);
        gateway = IPAddress(192, 168, 0, 255);
        subnet = IPAddress(255, 255, 0, 0);
        localPort = 8888;
    }

   
    void changeIP(IPAddress newIP, IPAddress newGateway, IPAddress newSubnet) {
        ip = newIP;
        gateway = newGateway;
        subnet = newSubnet;
    }

    void changeIP(IPAddress newIP) {
        ip = newIP;
    }

    IPAddress getIP() {
        return Ethernet.localIP();
    }

    
    bool connect() {
       
        if (isConnected) {
            disconnect();
        }

        Ethernet.begin(mac, ip, dns, gateway, subnet);

        
        if (Ethernet.linkStatus() == LinkOFF) {
            Serial.println("WARNING: Ethernet cable disconnected.");
        }

        udp.begin(localPort);
        isConnected = true;
        
        debug();
        return true;
    }

    void disconnect() {
        udp.stop();
        isConnected = false;
        Serial.println("[NET] Disconnected.");
    }


    int read() {
        if (!isConnected) return 0;

        int packetSize = udp.parsePacket();
        
        if (packetSize > 0) {
            lastSenderIP = udp.remoteIP();
            lastSenderPort = udp.remotePort();

            int bytesToRead = (packetSize > (int)INCOMING_PACKET_SIZE) ? (int)INCOMING_PACKET_SIZE : packetSize;
            
            udp.read(incoming_buffer, bytesToRead);
            return bytesToRead;
        }
        return 0; 
    }

    void sendReply() {
        if (!isConnected) return;
        
        if (lastSenderIP[0] != 0) { 
            udp.beginPacket(lastSenderIP, lastSenderPort);
            udp.write(outgoing_buffer, OUTGOING_PACKET_SIZE);
            udp.endPacket();
        }
    }

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

    incomingPacket* getIncomingPacket() {
        return (incomingPacket*)incoming_buffer;
    }

    outgoingPacket* getOutgoingPacket() {
        return (outgoingPacket*)outgoing_buffer;
    }

    void debug() {
        Serial.print("[NET] Active IP: ");
        Serial.println(Ethernet.localIP());
        Serial.printf("[NET] UDP Port: %u\n", localPort);
    }
};