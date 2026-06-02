#include <Arduino.h>
#include <Wire.h>
#include "Data_Structures.h"
#include "IMU_Control.h"
#include "ToF_Control.h"
#include "Button_Control.h"
#include "Motor_Control.h"

// --- Global Switches ---
#define USE_POWER_BUTTONS   true   // false = always on (easier for programming)
#define ENABLE_SERIAL_PRINT true   // false = disable for competition performance

// --- Hardware Pins ---
const int START_PIN  = 20;   // GP20
const int STOP_PIN   = 21;   // GP21
const int HEART_PIN  = 0;    // GP0 heartbeat LED
const int XSHUT_PIN  = 18;   // GP18 → VL53L4CX XSHUT

// --- I2C Bus Pins ---
// ToF VL53L4CX → Wire  (GP16=SDA blue, GP17=SCL yellow)
// IMU BNO085   → Wire1 (GP26=SDA blue, GP27=SCL yellow)
#define TOF_SDA 16
#define TOF_SCL 17
#define IMU_SDA 26
#define IMU_SCL 27

// --- Motor Speed ---
const int DRIVE_SPEED = 50;  // 70% of 255 — half speed for competition start

// --- Global Objects ---
IMU_Control    imu;
ToF_Control    tof;
Button_Control buttons(START_PIN, STOP_PIN);
Motor_Control  motors;

// --- State ---
bool          powerOn       = false;
bool          tofOK         = false;
bool          imuOK         = false;
bool          motorsRunning = false;
unsigned long loopCounter   = 0;

// --- Data Structs ---
struct_imu_data myIMU = {0};
struct_tof_data myToF = {0};

// --- Timing ---
unsigned long lastLogTime     = 0;
const long    LOG_INTERVAL_MS = 200;

// -------------------------------------------------------
// Hard reset VL53L4CX via XSHUT
// Equivalent to power cycling the sensor
// -------------------------------------------------------
void resetToF() {
    Serial.println("ToF: Hard reset via XSHUT...");
    pinMode(XSHUT_PIN, OUTPUT);
    digitalWrite(XSHUT_PIN, LOW);
    delay(10);
    digitalWrite(XSHUT_PIN, HIGH);
    delay(10);
    pinMode(XSHUT_PIN, INPUT);
}

void setup() {
    Serial.begin(115200);
    delay(2000);
    pinMode(HEART_PIN, OUTPUT);
    Serial.println("\n=== Robot Booting ===");

    // Init hardware
    buttons.begin();
    motors.begin();  // motors stopped on begin()

    powerOn = true;  // keep sensors and system active
    if (USE_POWER_BUTTONS) {
        Serial.println("Ready. Press GP20 to start motors, GP21 to stop motors.");
    } else {
        motors.setSpeed(DRIVE_SPEED, DRIVE_SPEED,
                        DRIVE_SPEED, DRIVE_SPEED);
        motorsRunning = true;
        Serial.println("Motors: running at 70% (button bypass)");
    }
    Serial.println();

    // ---- Init I2C buses ONCE — never inside modules ----
    Wire.setSDA(TOF_SDA);
    Wire.setSCL(TOF_SCL);
    Wire.begin();
    Wire.setClock(400000);
    Serial.println("Wire (ToF GP16/17) OK");

    Wire1.setSDA(IMU_SDA);
    Wire1.setSCL(IMU_SCL);
    Wire1.begin();
    Wire1.setClock(400000);
    Serial.println("Wire1 (IMU GP26/27) OK\n");

    // ---- Init ToF with XSHUT reset + 3 retries ----
    Serial.println("Initializing ToF...");
    for (int i = 0; i < 3; i++) {
        resetToF();
        delay(100);
        if (tof.begin(Wire)) {
            tofOK = true;
            Serial.println("ToF OK\n");
            break;
        }
        Serial.print("ToF attempt "); Serial.print(i + 1);
        Serial.println(" failed, retrying...");
        delay(500);
    }
    if (!tofOK) Serial.println("WARNING: ToF unavailable — continuing without it\n");

    // ---- Init IMU with 3 retries ----
    Serial.println("Initializing IMU...");
    for (int i = 0; i < 3; i++) {
        if (imu.begin(Wire1)) {
            imuOK = true;
            Serial.println("IMU OK\n");
            break;
        }
        Serial.print("IMU attempt "); Serial.print(i + 1);
        Serial.println(" failed, retrying...");
        delay(500);
    }
    if (!imuOK) Serial.println("WARNING: IMU unavailable — continuing without it\n");

    Serial.print("=== System ready | IMU:");
    Serial.print(imuOK ? "OK" : "FAIL");
    Serial.print(" ToF:");
    Serial.println(tofOK ? "OK" : "FAIL");
    Serial.println();
}

void loop() {
    unsigned long loopStartTime = millis();
    loopCounter++;
    unsigned long currentTime = millis();

    // --- Heartbeat LED blink every 1 second ---
    static unsigned long lastBlinkTime = 0;
    if (currentTime - lastBlinkTime >= 1000) {
        digitalWrite(HEART_PIN, !digitalRead(HEART_PIN));
        lastBlinkTime = currentTime;
    }

    // --- Poll sensors ---
    if (imuOK) imu.read(myIMU);
    if (tofOK) tof.read(myToF);

    // --- Start button (motor control only) ---
    if (USE_POWER_BUTTONS && buttons.isStartPressed() && !motorsRunning) {
        motors.setSpeed(DRIVE_SPEED, DRIVE_SPEED,
                        DRIVE_SPEED, DRIVE_SPEED);
        motorsRunning = true;
        Serial.println("Start pressed — motors running at 70%");
    }

    // --- Stop button (motor control only) ---
    if (USE_POWER_BUTTONS && buttons.isStopPressed() && motorsRunning) {
        Serial.println("Stop pressed — stopping motors...");
        motors.stop();
        motorsRunning = false;
        Serial.println("Motors stopped");
    }

    // --- Serial telemetry ---
    if (currentTime - lastLogTime >= LOG_INTERVAL_MS) {
        lastLogTime = currentTime;
        if (ENABLE_SERIAL_PRINT) {
            Serial.print("Loop: ");    Serial.print(loopCounter);
            Serial.print(" | Time: "); Serial.print(millis() - loopStartTime);
            Serial.print("ms | IMU:"); Serial.print(imuOK ? "OK" : "--");
            Serial.print(" ToF:");     Serial.println(tofOK ? "OK" : "--");

            if (imuOK) {
                Serial.print("IMU Yaw: ");     Serial.print(myIMU.yaw,    2);
                Serial.print(" Pitch: ");      Serial.print(myIMU.pitch,   2);
                Serial.print(" Roll: ");       Serial.print(myIMU.roll,    2);
                Serial.print(" Acc: ");
                Serial.print(myIMU.accX, 2);  Serial.print(",");
                Serial.print(myIMU.accY, 2);  Serial.print(",");
                Serial.print(myIMU.accZ, 2);
                Serial.print(" Gyro: ");
                Serial.print(myIMU.gyroX, 2); Serial.print(",");
                Serial.print(myIMU.gyroY, 2); Serial.print(",");
                Serial.print(myIMU.gyroZ, 2);
                Serial.print(" Acc#: ");       Serial.println(myIMU.accuracy);
            }

            if (tofOK) {
                Serial.print("ToF count: "); Serial.print(myToF.count);
                if (myToF.count > 0) {
                    Serial.print(" Dist=");
                    Serial.print(myToF.targets[0].distance);
                    Serial.print("mm St=");
                    Serial.print(myToF.targets[0].status);
                }
                Serial.println();
            }
            Serial.println();
        }
    }
}