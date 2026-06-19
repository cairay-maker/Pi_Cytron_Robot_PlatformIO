#include "IMU_Control.h"

IMU_Control::IMU_Control() {}

bool IMU_Control::begin(TwoWire& wire) {
    Serial.println("IMU: begin_I2C(0x4A)...");
    if (!_bno.begin_I2C(0x4A, &wire)) {
        Serial.println("IMU: begin_I2C failed!");
        return false;
    }
    _bno.enableReport(SH2_ROTATION_VECTOR);
    _bno.enableReport(SH2_ACCELEROMETER);
    _bno.enableReport(SH2_GYROSCOPE_CALIBRATED);
    _bno.enableReport(SH2_MAGNETIC_FIELD_CALIBRATED);
    Serial.println("IMU: Ready");
    return true;
}

bool IMU_Control::read(struct_imu_data& data) {
    sh2_SensorValue_t event;
    if (!_bno.getSensorEvent(&event)) return false;

    switch (event.sensorId) {
        case SH2_ROTATION_VECTOR: {
            float qr = event.un.rotationVector.real;
            float qi = event.un.rotationVector.i;
            float qj = event.un.rotationVector.j;
            float qk = event.un.rotationVector.k;
            
            // Convert Quaternion to Euler Angles (Degrees)
            data.yaw      = atan2(2.0 * (qr * qk + qi * qj), 1.0 - 2.0 * (qj * qj + qk * qk)) * 180.0 / PI;
            data.pitch    = asin(2.0 * (qr * qj - qk * qi)) * 180.0 / PI;
            data.roll     = atan2(2.0 * (qr * qi + qj * qk), 1.0 - 2.0 * (qi * qi + qj * qj)) * 180.0 / PI;
            data.accuracy = event.un.rotationVector.accuracy;
            break;
        }
        case SH2_ACCELEROMETER:
            data.accX = event.un.accelerometer.x;
            data.accY = event.un.accelerometer.y;
            data.accZ = event.un.accelerometer.z;
            break;
        case SH2_GYROSCOPE_CALIBRATED:
            data.gyroX = event.un.gyroscope.x;
            data.gyroY = event.un.gyroscope.y;
            data.gyroZ = event.un.gyroscope.z;
            break;
    }
    return true;
}