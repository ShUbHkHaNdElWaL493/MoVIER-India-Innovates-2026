
#pragma once
//NOTE
// If you use EN: You must keep BRK connected to COM permanently.
// If you use BRK: You must keep EN connected to COM permanently
// we r uning brk for now

#include <Arduino.h>
#include "encoderFeedback.hpp"

class Wheel{
    public:
        uint8_t pwm_pin, dir_pin, brk_pin,hw_pin,hv_pin,hu_pin;
        float kp, ki, kd;
        encoder enc;
        int factor; 
            float step = 0;
            float integral_err = 0.0;
            float prev_error = 0.0; 
        float max_integral = 50.0;
        float total_pwm = 0;

        Wheel(uint8_t pwm_pin, uint8_t dir_pin, uint8_t brk_pin, uint8_t hw_pin, uint8_t hv_pin, uint8_t hu_pin, uint8_t Pulse_per_motor_revolution,int factor) 
        : enc(hw_pin, hv_pin, hu_pin, Pulse_per_motor_revolution,factor)
        {
            this->pwm_pin = pwm_pin;
            this->dir_pin = dir_pin;
            this->brk_pin = brk_pin;
            this->factor = -1*factor;
            pinMode(pwm_pin, OUTPUT);
            pinMode(dir_pin, OUTPUT);
            pinMode(brk_pin, OUTPUT);

        }



        void moveOpenLoop(int pwm){
            if (pwm>255){
                pwm=150;
            }else if(pwm<-255){
                pwm=-150;
            }
            if (pwm>0){
                digitalWrite(dir_pin,LOW);
                digitalWrite(brk_pin,LOW);
            }else if(pwm<0){
                digitalWrite(dir_pin,HIGH);
                digitalWrite(brk_pin,LOW);
            }else{
                pwm=0;
                digitalWrite(brk_pin,HIGH);
            }
                analogWrite(pwm_pin,abs(pwm));
                // Serial.println(abs(pwm));
        }


        void moveClosedLoop(float target_rpm) {
            // 1. Handle the Stop condition

            if (target_rpm == 0) {
                total_pwm = 0;
                moveOpenLoop(0);
                return;
            }

            // 2. Get current velocity
            float current_rpm = enc.getRPM();
            // Serial.print("Current RPM || ");
            // Serial.println(current_rpm);
            // 3. Simple Incremental Logic (as requested)
            // Using a deadband of 1.2 RPM
            if (current_rpm < target_rpm - 1.2) {
                total_pwm += 1.0; // Equivalent to left_effort++
            } 
            else if (current_rpm > target_rpm + 1.2) {
                total_pwm -= 1.0; // Equivalent to left_effort--
            }

            // Serial.print("total_pwm || ");
            // Serial.println(total_pwm);
            // // 4. Constrain PWM to valid hardware limits
            if (total_pwm > 255) total_pwm = 255;
            if (total_pwm < -255) total_pwm = -255;

            // // 5. Apply the effort with the wheel's specific direction factor
            moveOpenLoop((int)total_pwm * this->factor);
        }
    };