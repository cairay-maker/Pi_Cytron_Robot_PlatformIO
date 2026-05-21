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
        case SH2_ROTATION_VECTOR:
            data.yaw      = event.un.rotationVector.real;
            data.pitch    = event.un.rotationVector.i;
            data.roll     = event.un.rotationVector.j;
            data.accuracy = event.un.rotationVector.accuracy;
            break;
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