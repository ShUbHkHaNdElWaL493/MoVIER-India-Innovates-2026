#define MODE_SERIAL

#include <Arduino.h>
#include "wheel.hpp"
#include "imu.hpp"
#include <IWatchdog.h>

#ifdef MODE_NETWORK
#include "network.hpp"
#endif

#ifdef MODE_SERIAL
#include "serial.hpp"
#endif

//left_pins
uint8_t right_pwm = D6;
uint8_t right_dir = D7;
uint8_t right_brk = A4;
uint8_t right_enc_hw =  D15 ;
uint8_t right_enc_hv =  D14 ;
uint8_t right_enc_hu = A5;
float right_cpr = 90;

//right_pins
uint8_t left_pwm = D9;
uint8_t left_dir = D10;
uint8_t left_brk = A2;
uint8_t left_enc_hw = A0;
uint8_t left_enc_hv = A3;
uint8_t left_enc_hu = A1;
float left_cpr = 90;

//object declaration
Wheel wheel_left(left_pwm,left_dir,left_brk,left_enc_hw,left_enc_hv,left_enc_hu,left_cpr,-1);
Wheel wheel_right(right_pwm,right_dir,right_brk,right_enc_hw,right_enc_hv,right_enc_hu,right_cpr,1);

imu Imu1(PB11, PB10);

#ifdef MODE_NETWORK
NetworkManager manager;
#endif

#ifdef MODE_SERIAL
SerialManager manager;
#endif

//constants
const uint32_t CONTROL_LOOP_US = 5000; 
const uint32_t SAFETY_TIMEOUT_MS = 500; 
static uint32_t last_control_time = 0;
static uint32_t last_packet_time = 0;
uint32_t sequence_num = 0;


//interrupt service routines
void isr_left() {
    wheel_left.enc.countPulse();
}
void isr_right() {
    wheel_right.enc.countPulse();
}


void setup()
{

  attachInterrupt(digitalPinToInterrupt(left_enc_hw), isr_left, CHANGE);
  attachInterrupt(digitalPinToInterrupt(left_enc_hv), isr_left, CHANGE);
  attachInterrupt(digitalPinToInterrupt(left_enc_hu), isr_left, CHANGE);
  attachInterrupt(digitalPinToInterrupt(right_enc_hw), isr_right, CHANGE);
  attachInterrupt(digitalPinToInterrupt(right_enc_hv), isr_right, CHANGE);
  attachInterrupt(digitalPinToInterrupt(right_enc_hu), isr_right, CHANGE);

  Serial.begin(115200);
  if (!Imu1.begin()) {
      Serial.println("IMU Initialization Failed!");
      // Hardware will not hang here now unless you add a while(1)
  }
  #ifdef MODE_NETWORK
  while(!manager.connect())
  {
    Serial.println("Network connection failed. Halting.");
    delay(500);
  }
  Serial.println("Network connection done. Halting.");
//   IWatchdog.begin(2000000);
  #endif
  last_packet_time = millis();

} 
uint32_t nw = 0;


// void loop(){
     
//         wheel_left.moveOpenLoop(45);
//         wheel_right.moveOpenLoop(45);
    
    // // wheel_right.moveClosedLoop(40);
    // nw = millis();
    // for(int i =0; i<200000; i++){
    //     wheel_left.moveClosedLoop(20);
    //  wheel_right.moveClosedLoop(-20);
    //     Serial.print("Right enc");  
    //     Serial.println(wheel_right.enc.getRPM());
    // delay(50);

    // }
    //  wheel_left.moveOpenLoop(0);
    //  wheel_right.moveOpenLoop(0);

    // // wheel_right.moveClosedLoop(40);
    // nw = millis();
    // for(int i =0; i<20; i++){
    //     Serial.print("Left enc");
    //     Serial.println(wheel_left.enc.getRPM());
    //     Serial.print("Right enc");  
    //     Serial.println(wheel_right.enc.getRPM());
    // delay(500);

    // } wheel_left.moveOpenLoop(30);
    //  wheel_right.moveOpenLoop(30);

    // // wheel_right.moveClosedLoop(40);
    // nw = millis();
    // for(int i =0; i<20; i++){
    //     Serial.print("Left enc");
    //     Serial.println(wheel_left.enc.getRPM());
    //     Serial.print("Right enc");  
    //     Serial.println(wheel_right.enc.getRPM());
    // delay(500);

    // }
// }

// void loop() {
//     uint32_t now_us = micros();

//     // 1. Send feedback to PC every 10ms
//     if (now_us - last_control_time >= CONTROL_LOOP_US) {
//         last_control_time = now_us;
//         outgoingPacket* out = manager.getOutgoingPacket();
        
//         // --- DUMMY IMU DATA START ---
//         static float dummy_val = 0.0f;
//         dummy_val += 0.1f;
//         if (dummy_val > 100.0f) dummy_val = 0.0f;

//         imuPacket dummy_imu;
//         dummy_imu.qx = 1.11f; dummy_imu.qy = 2.22f; dummy_imu.qz = 3.33f; dummy_imu.qw = 4.44f;
//         dummy_imu.ax = dummy_val; // This will ramp up in your Python script
//         dummy_imu.ay = 5.55f;
//         dummy_imu.az = 6.66f;
//         dummy_imu.gx = 7.77f; dummy_imu.gy = 8.88f; dummy_imu.gz = 9.99f;
//         // --- DUMMY IMU DATA END ---

//         out->start = START_BYTE; // 0xA5
//         out->seq = sequence_num;          
//         out->status = 0; 
        
//         // Assign the dummy struct directly
//         out->imu_data = dummy_imu; 
        
//         out->enc_count_left = wheel_left.enc.getCount();
//         out->enc_count_right = wheel_right.enc.getCount();
//         out->end = END_BYTE;    // 0x5A
        
//         manager.sendReply();
//     }

//     // 2. Read commands from PC
//     if(manager.read() > 0) {
//         incomingPacket* in = manager.getIncomingPacket();
//         sequence_num = in->seq;
//         wheel_left.moveClosedLoop(in->cmd_vel_left);
//         wheel_right.moveClosedLoop(in->cmd_vel_right);
//     }
// }

void loop() {
    uint32_t now_us = micros();

    if (now_us - last_control_time >= CONTROL_LOOP_US) {
        last_control_time = now_us;
        outgoingPacket* out = manager.getOutgoingPacket();
        
        out->start = START_BYTE;
        out->seq = sequence_num;
        out->status = 0; 
        
        // This now copies the persistent values from the IMU class memory
        out->imu_data = *(Imu1.getImuReading()); 
        
        out->enc_count_left = wheel_left.enc.getCount();
        out->enc_count_right = wheel_right.enc.getCount();
        out->end = END_BYTE;
        
        manager.sendReply();
    }

    // 2. Read commands from PC
    if(manager.read() > 0) {
        incomingPacket* in = manager.getIncomingPacket();
        
        sequence_num = in->seq;
        // if (in->cmd_vel_left*in->cmd_vel_right>0){
        //   if (in->cmd_vel_left>in->cmd_vel_right){
        //     wheel_left.moveOpenLoop(50);
        //     wheel_right.moveOpenLoop(50);
        //   }else{
        //     wheel_left.moveOpenLoop(-50);
        //     wheel_right.moveOpenLoop(-50);
        //   }
        // }else{
        // Execute the motors
        wheel_left.moveClosedLoop(in->cmd_vel_left);
        wheel_right.moveClosedLoop(in->cmd_vel_right);
        // }
    }
}