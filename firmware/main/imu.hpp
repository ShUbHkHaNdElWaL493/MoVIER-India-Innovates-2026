#pragma once 
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BNO08x.h>

#define BNO08X_RESET -1 

struct imuPacket {
    float qx, qy, qz, qw;
    float ax, ay, az;
    float gx, gy, gz;
};

class imu {
    public:
        imu(uint8_t sda, uint8_t scl) : SDA_PIN(sda), SCL_PIN(scl) {
            // Initialize struct with default "neutral" values
            imudata = {0, 0, 0, 1.0, 0, 0, 0, 0, 0, 0};
        }

        bool begin() {
            Wire.setSCL(SCL_PIN); 
            Wire.setSDA(SDA_PIN);
            Wire.begin(); 
            Wire.setClock(400000); // Fast I2C

            if (!bno08x.begin_I2C()) {
                return false; 
            }
            enableReports();
            return true;
        }

        void enableReports() {
            // Use Linear Acceleration to match your successful test code
            bno08x.enableReport(SH2_ROTATION_VECTOR, 10000);
            bno08x.enableReport(SH2_LINEAR_ACCELERATION, 10000);
            bno08x.enableReport(SH2_GYROSCOPE_CALIBRATED, 10000);
        }

        imuPacket* getImuReading() {
            if (bno08x.wasReset()) {
                enableReports();
            }
            
            sh2_SensorValue_t sensorValue;
            // IMPORTANT: Use a while loop to empty the sensor queue
            // This ensures we capture EVERY update since the last poll
            while (bno08x.getSensorEvent(&sensorValue)) {
                switch (sensorValue.sensorId) {
                    case SH2_ROTATION_VECTOR:
                        imudata.qx = sensorValue.un.rotationVector.i;
                        imudata.qy = sensorValue.un.rotationVector.j;
                        imudata.qz = sensorValue.un.rotationVector.k;
                        imudata.qw = sensorValue.un.rotationVector.real;
                        break;
                    case SH2_LINEAR_ACCELERATION:
                        imudata.ax = sensorValue.un.linearAcceleration.x;
                        imudata.ay = sensorValue.un.linearAcceleration.y;
                        imudata.az = sensorValue.un.linearAcceleration.z;
                        break;
                    case SH2_GYROSCOPE_CALIBRATED:
                        imudata.gx = sensorValue.un.gyroscope.x;
                        imudata.gy = sensorValue.un.gyroscope.y;
                        imudata.gz = sensorValue.un.gyroscope.z;
                        break;
                }
            }
            return &imudata; // Return the address of the persistent data
        }
        
    private:
        uint8_t SDA_PIN, SCL_PIN;
        Adafruit_BNO08x bno08x = Adafruit_BNO08x(BNO08X_RESET);
        imuPacket imudata; 
};