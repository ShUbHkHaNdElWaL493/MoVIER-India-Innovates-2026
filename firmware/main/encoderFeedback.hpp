#include <Arduino.h>

class encoder {
    public:
        float rotations;    
        float current_rpm = 0;

        encoder(uint8_t HW_PIN, uint8_t HV_PIN, uint8_t HU_PIN, uint8_t Pulse_per_motor_revolution, int factor) {
            hw_pin = HW_PIN;
            hv_pin = HV_PIN;
            hu_pin = HU_PIN;
            this->factor = factor;
            pulse_per_motor_revolution = Pulse_per_motor_revolution;
            pinMode(HW_PIN, INPUT_PULLUP); 
            pinMode(HV_PIN, INPUT_PULLUP); 
            pinMode(HU_PIN, INPUT_PULLUP); 
            // Initialize lastState to avoid a jump on the first interrupt
            lastState = (digitalRead(hu_pin) << 2) | (digitalRead(hv_pin) << 1) | digitalRead(hw_pin);
        }

        float getRPM() {
            unsigned long currentTime = millis();
            if (currentTime - lastTime >= sampleInterval) {
                noInterrupts();
                int32_t currentPulses = pulseCount; // Use signed int
                interrupts();

                float deltaTicks = (float)(currentPulses - lastEncoderValue);
                // Use the actual elapsed time for more accurate RPM
                current_rpm = (deltaTicks / (float)pulse_per_motor_revolution) * (60000.0 / (currentTime - lastTime));

                lastEncoderValue = currentPulses;   
                lastTime = currentTime;
                rotations = (float)currentPulses / pulse_per_motor_revolution;
            }
            return factor*current_rpm; 
        }

        int32_t getCount() {
            return factor*pulseCount;
        }

        void countPulse() {
            int valA = digitalRead(hw_pin);
            int valB = digitalRead(hv_pin);
            int valC = digitalRead(hu_pin);
            
            // Align bit order with your working test script
            int currentState = (valC << 2) | (valB << 1) | valA;

            if (currentState != lastState) {
                if ((lastState == 5 && currentState == 1) || 
                    (lastState == 1 && currentState == 3) || 
                    (lastState == 3 && currentState == 2) ||
                    (lastState == 2 && currentState == 6) ||
                    (lastState == 6 && currentState == 4) ||
                    (lastState == 4 && currentState == 5)) {
                    pulseCount++;
                } else {
                    pulseCount--;
                }
                lastState = currentState; // No longer static; belongs to the instance
            }
        }
    
    private:
        uint8_t hu_pin, hv_pin, hw_pin;
        uint8_t pulse_per_motor_revolution;
        const unsigned long sampleInterval = 20; 
        volatile int32_t pulseCount = 0;      // Changed to signed
        volatile int32_t lastEncoderValue = 0; // Changed to signed
        unsigned long lastTime = 0;
        int lastState = 0; // Instance-specific state
        int factor = 1;
};