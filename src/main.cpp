#include <Arduino.h>
#include <Wire.h>
#include "Data_Structures.h"
#include "IMU_Control.h"
#include "ToF_Control.h"
#include "Button_Control.h"
#include "Motor_Control.h"
#include "Robot_Status.h"
#include "Drive_Controller.h"
#include "Ball_Handler.h"
#include "Simulation_Map.h" // Updated naming convention target

// =======================================================
// CONFIGURATION MODES
// =======================================================
#define SIMULATION_MODE     true   // true = Runs virtual timeline sandbox; false = Live track execution
#define USE_POWER_BUTTONS   true   // true = Must press GP20 to start motor routines
#define ENABLE_SERIAL_PRINT true   // true = Output telemetry data to console

// --- Hardware Pins ---
const int START_PIN = 20;   // GP20 Start Button
const int STOP_PIN  = 21;   // GP21 Stop Button
const int HEART_PIN = 0;    // GP0 Heartbeat onboard LED
const int XSHUT_PIN = 18;   // GP18 Hardware Shutdown pin for VL53L4CX

// --- I2C Bus Pins ---
#define TOF_SDA 16
#define TOF_SCL 17
#define IMU_SDA 26
#define IMU_SCL 27

// =======================================================
// GLOBAL SYSTEM OBJECTS
// =======================================================
IMU_Control    imu;
ToF_Control    tof;
Button_Control buttons(START_PIN, STOP_PIN);
Motor_Control  motors;

// Architecture Modules
Robot_Status     status;
Drive_Controller driver(motors);
Ball_Handler     ballHandler;
Simulation_Map   sim(status); // Instantiated cleanly with singular name

// --- Core System State ---
bool systemRunning = false;
bool imuOK         = false;
bool tofOK         = false;
unsigned long simStartTime = 0;

// --- Data Buffers ---
struct_imu_data myIMU = {0};
struct_tof_data myToF = {0};

// --- Telemetry Log Interval ---
unsigned long lastLogTime     = 0;
const long    LOG_INTERVAL_MS = 250;

// -------------------------------------------------------
// Hardware Power Cycle for VL53L4CX ToF Boot Safety
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

// =======================================================
// SYSTEM SETUP
// =======================================================
void setup() {
    Serial.begin(115200);
    delay(2000); // Guard window for serial monitor to connect
    pinMode(HEART_PIN, OUTPUT);
    Serial.println("\n=== Team Leli Robot Booting ===");

    // 1. Fire up baseline UI mechanical structures
    buttons.begin();
    motors.begin();  // Standard safety stop applied automatically
    status.begin();
    ballHandler.begin();

    // 2. Set indicators to visual diagnostic color (LED 1: Yellow)
    status.changeState(SystemState::BOOT_DIAGNOSTIC);
    status.update();

    // 3. Initialize I2C Bus Lanes Once
    Wire.setSDA(TOF_SDA);
    Wire.setSCL(TOF_SCL);
    Wire.begin();
    Wire.setClock(400000);
    Serial.println("Wire Bus (ToF GP16/17) Active");

    Wire1.setSDA(IMU_SDA);
    Wire1.setSCL(IMU_SCL);
    Wire1.begin();
    Wire1.setClock(400000);
    Serial.println("Wire1 Bus (IMU GP26/27) Active\n");

    // 4. ToF Boot Checklist Sequence (3 Retries)
    Serial.println("Initializing ToF Sensor Module...");
    for (int i = 0; i < 3; i++) {
        resetToF();
        delay(100);
        if (tof.begin(Wire)) {
            tofOK = true;
            Serial.println("ToF Init Match -> SUCCESS\n");
            break;
        }
        Serial.print("ToF Attempt "); Serial.print(i + 1); Serial.println(" Failed...");
        delay(400);
    }

    // 5. IMU Boot Checklist Sequence (3 Retries)
    Serial.println("Initializing IMU Sensor Module...");
    for (int i = 0; i < 3; i++) {
        if (imu.begin(Wire1)) {
            imuOK = true;
            Serial.println("IMU Init Match -> SUCCESS\n");
            break;
        }
        Serial.print("IMU Attempt "); Serial.print(i + 1); Serial.println(" Failed... Retrying");
        delay(400);
    }

    // 6. Final Boot Health Diagnostics Evaluation
    if (imuOK && tofOK) {
        status.changeState(SystemState::IDLE_STANDBY, SubStatus::NONE_OK); // LED 1: Blue, LED 2: Off
        Serial.println("=== System Clear: All Peripherals Online ===");
    } else {
        // Error Overlay Routing: Keeps system running for Simulation test bypass
        if (!imuOK) status.changeState(SystemState::CRITICAL_FAULT, SubStatus::ERROR_IMU);
        if (!tofOK) status.changeState(SystemState::CRITICAL_FAULT, SubStatus::ERROR_TOF);
        Serial.println("WARNING: Initializing with limited hardware peripherals.");
    }
    
    if (!USE_POWER_BUTTONS) {
        systemRunning = true;
        simStartTime = millis();
        #if (SIMULATION_MODE == true)
            sim.initScenario(TestScenario::FULL_COMPETITION_RUN, simStartTime);
        #else
            status.changeState(SystemState::LINE_TRACING, SubStatus::TRACE_NORMAL);
        #endif
        Serial.println("Button bypass active: Engine sequences running.");
    }
}

// =======================================================
// MAIN EXECUTION LOOP
// =======================================================
void loop() {
    unsigned long currentTime = millis();
    status.update(); // Maintain independent status LED rhythms

    // --- Core Heartbeat LED Ticker (1Hz) ---
    static unsigned long lastBlinkTime = 0;
    if (currentTime - lastBlinkTime >= 1000) {
        digitalWrite(HEART_PIN, !digitalRead(HEART_PIN));
        lastBlinkTime = currentTime;
    }

    // --- Poll Physical Control Interfaces ---
    if (USE_POWER_BUTTONS) {
        if (buttons.isStartPressed() && !systemRunning) {
            systemRunning = true;
            simStartTime = currentTime; // Zero out timeline clock framework anchor
            
            #if (SIMULATION_MODE == true)
                // CHOOSE TEST SCENARIO HERE: 
                // Options: TestScenario::FULL_COMPETITION_RUN, TestScenario::RAMP_CHALLENGE, TestScenario::RESCUE_ZONE_BALL_SORT
                sim.initScenario(TestScenario::FULL_COMPETITION_RUN, simStartTime);
                Serial.println("START pressed -> Execution Sandbox Live.");
            #else
                status.changeState(SystemState::LINE_TRACING, SubStatus::TRACE_NORMAL);
                Serial.println("START pressed -> Running Real Track Routine.");
            #endif
        }

        if (buttons.isStopPressed() && systemRunning) {
            systemRunning = false;
            status.changeState(SystemState::IDLE_STANDBY, SubStatus::NONE_OK);
            motors.stop();
            Serial.println("STOP pressed -> Hardware execution halted safely.");
        }
    }

    // --- Active Logic Controller Path Routing ---
    if (systemRunning) {
        #if (SIMULATION_MODE == true)
            // ===================================================
            // SANDBOX EXECUTION ROUTE (Bypasses Sensor Reading)
            // ===================================================
            sim.update(currentTime);
            
        #else
            // ===================================================
            // REAL MAP RUN ROUTE (Active Hardware Interception)
            // ===================================================
            if (imuOK) imu.read(myIMU);
            if (tofOK) tof.read(myToF);

            // Real track evaluation loops and Python Serial parsers interface here:
            // Example:
            // if (Serial.available()) { ... status.changeState(...) }
        #endif

        // --- Hardware Muscle Execution Handlers ---
        driver.executeTrajectory(status.getState(), status.getSubStatus());
        ballHandler.processRescueTask(status.getSubStatus());

    } else {
        // Enforce safe static posture if process loop is deactivated
        motors.stop();
    }

    // --- Serial Stream Monitor Telemetry Output ---
    if (ENABLE_SERIAL_PRINT && (currentTime - lastLogTime >= LOG_INTERVAL_MS)) {
        lastLogTime = currentTime;
        
        Serial.print("MODE: [");
        if (SIMULATION_MODE) Serial.print("SIMULATION");
        else Serial.print("LIVE_TRACK");
        Serial.print("] | Master State ID: ");
        Serial.print((int)status.getState());
        Serial.print(" | Sub-Action Focus Target: ");
        Serial.println((int)status.getSubStatus());

        #if (SIMULATION_MODE == false)
            // Output sensor matrices only during live execution tracks to save CPU execution cycles
            if (imuOK) {
                Serial.print("   -> IMU Yaw: "); Serial.print(myIMU.yaw, 2);
                Serial.print(" Pitch: ");        Serial.print(myIMU.pitch, 2);
                Serial.print(" AccY: ");         Serial.println(myIMU.accY, 2);
            }
            if (tofOK && myToF.count > 0) {
                Serial.print("   -> ToF Tracking Distance: "); 
                Serial.print(myToF.targets[0].distance); 
                Serial.println("mm");
            }
        #endif
    }
}