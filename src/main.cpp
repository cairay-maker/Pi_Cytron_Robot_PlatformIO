#include <Arduino.h>
#include <Wire.h>
#include "Data_Structures.h"
#include "IMU_Control.h"
#include "ToF_Control.h"
#include "Button_Control.h"
#include "Motor_Control.h"
#include "Robot_Status.h"
#include "Drive_Controller.h"
#include "Camera_Mount.h"
#include "Ball_Handler.h"
#include "Simulation_Map.h" 
#include "Serial_Control.h"
#include "Config.h"
#include "RPi_Interface.h"
#include "Line_Processor.h"

// =======================================================
// CONFIGURATION MODES
// =======================================================
bool cfgSimMode            = DEFAULT_SIM_MODE;
bool cfgRequireStartButton = DEFAULT_REQUIRE_START_BUTTON;
bool cfgEnablePrint        = DEFAULT_ENABLE_PRINT;
bool cfgMotorEnable        = DEFAULT_MOTOR_ENABLE;
bool cfgAvoidRight         = DEFAULT_AVOID_RIGHT;


// =======================================================
// GLOBAL SYSTEM OBJECTS
// =======================================================
IMU_Control    imu;
ToF_Control    tof;
Button_Control buttons(START_PIN, STOP_PIN);
Motor_Control  motors;

// Architecture Modules
Robot_Status     status;
Drive_Controller driver(motors, imu);
Camera_Mount     camMount;     // System Area: Owned by Lillian
Ball_Handler     ballHandler;  // System Area: Owned by Lexi & Daniel
Simulation_Map   sim(status); 
Serial_Control   serialControl(camMount, ballHandler, motors, cfgSimMode, cfgRequireStartButton, cfgEnablePrint, cfgMotorEnable);

// Vision Integration
RPi_Interface    rpiInterface;
Line_Processor   lineProcessor(rpiInterface);

// --- Core System State ---
bool systemRunning = false;
bool isPaused      = false;
bool imuOK         = false;
bool tofOK         = false;
unsigned long simStartTime = 0;
unsigned long simPauseOffset = 0;

// --- Data Buffers ---
struct_imu_data myIMU = {0};
struct_tof_data myToF = {0};
unsigned long lastToFSuccessTime = 0;
unsigned long lastAvoidanceEndTime = 0;

// --- Telemetry Log Interval ---
unsigned long lastLogTime     = 0;
const long    LOG_INTERVAL_MS = 250;

// --- Telemetry serial log counter ---
unsigned long serialLogCounter = 0;

// -------------------------------------------------------
// Hardware Power Cycle for VL53L4CX ToF Boot Safety
// -------------------------------------------------------
void resetToF() {
    Serial.println("ToF: Hard reset via XSHUT...");
    pinMode(XSHUT_PIN, OUTPUT);
    digitalWrite(XSHUT_PIN, LOW);
    delay(10);
    digitalWrite(XSHUT_PIN, HIGH);
    // Increase delay to give the sensor firmware plenty of time to boot, 
    // and keep the pin as an active HIGH output so it strongly resists electrical noise.
    delay(50); 
}

// =======================================================
// SYSTEM SETUP
// =======================================================
void setup() {
    Serial.begin(115200);
    
    // Smart guard window: Wait for Serial Monitor to connect, with a 5-second timeout
    // so it doesn't freeze the robot if you power it on from a battery without a USB cable.
    unsigned long startWait = millis();
    while (!Serial && millis() - startWait < 5000) { delay(10); }
    
    pinMode(HEART_PIN, OUTPUT);
    Serial.println("\n=== Team Leli Robot Booting ===");

    // 1. Fire up PANline UI mechanical structures
    buttons.begin();
    motors.begin();  // Standard safety stop applied automatically
    status.begin();
    camMount.begin();    
    ballHandler.begin();
    serialControl.begin();
    rpiInterface.begin();
    
    // Link modules
    serialControl.setLineProcessorRef(&lineProcessor);
    serialControl.setRPiInterfaceRef(&rpiInterface);

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
        // BYPASS: Temporarily skip ToF hardware init to prevent I2C hangs
        if (false && tof.begin(Wire)) {
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
        // BYPASS: Temporarily skip IMU hardware init to prevent I2C hangs
        if (false && imu.begin(Wire1)) {
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
    
    // Start the watchdog clock right before entering the main loop
    lastToFSuccessTime = millis();
    
    if (!cfgRequireStartButton) {
        systemRunning = true;
        simStartTime = millis();
        if (cfgSimMode) {
            sim.initScenario(TestScenario::FULL_COMPETITION_RUN, simStartTime);
        } else {
            status.changeState(SystemState::LINE_TRACING, SubStatus::TRACE_NORMAL);
        }
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
    if (cfgRequireStartButton) {
        bool physStartPressed = buttons.isStartPressed();
        bool physStopPressed  = buttons.isStopPressed();
        bool virtStartPressed = serialControl.isVirtualStartPressed();
        bool virtStopPressed  = serialControl.isVirtualStopPressed();

        // Use STOP button during standby to toggle Left/Right avoidance
        if ((physStopPressed || virtStopPressed) && !systemRunning) {
            cfgAvoidRight = !cfgAvoidRight;
            Serial.print(">>> Avoidance Direction Toggled! Now dodging: ");
            Serial.println(cfgAvoidRight ? "RIGHT" : "LEFT");
        }

        // START / RESUME LOGIC
        if ((physStartPressed || virtStartPressed) && !systemRunning) {
            systemRunning = true;
            tof.resetConfidence();
            
            if (virtStartPressed && isPaused) {
                // Resume from pause via RUN:ON
                isPaused = false;
                simStartTime = currentTime - simPauseOffset;
                Serial.println("RUN:ON received -> Resuming execution.");
            } else {
                // Fresh start via physical GP20 Start, or fresh RUN:ON
                isPaused = false;
                simPauseOffset = 0;
                simStartTime = currentTime; // Zero out timeline clock framework anchor
                Serial.println(physStartPressed ? "START pressed -> Fresh Run Live." : "RUN:ON received -> Fresh Run Live.");
            }

            if (cfgSimMode) {
                // CHOOSE TEST SCENARIO HERE: 
                // Options: TestScenario::FULL_COMPETITION_RUN, TestScenario::RAMP_CHALLENGE, TestScenario::RESCUE_ZONE_BALL_SORT
                sim.initScenario(TestScenario::FULL_COMPETITION_RUN, simStartTime);
            } else {
                status.changeState(SystemState::LINE_TRACING, SubStatus::TRACE_NORMAL);
            }
        }

        // KILL / PAUSE LOGIC
        if ((physStopPressed || virtStopPressed) && systemRunning) {
            systemRunning = false;
            motors.stop();
            status.changeState(SystemState::IDLE_STANDBY, SubStatus::NONE_OK);
            
            if (physStopPressed) {
                // Hard kill via GP21
                isPaused = false;
                simPauseOffset = 0;
                Serial.println("STOP pressed -> Hardware execution killed safely.");
            } else if (virtStopPressed) {
                // Pause via RUN:OFF
                isPaused = true;
                simPauseOffset = currentTime - simStartTime;
                Serial.println("RUN:OFF received -> Hardware execution PAUSED.");
            }
        }
    }

    // --- On-The-Fly Serial Command Interceptor for Diagnostics ---
    serialControl.update();

    // --- Active Sensor Harvesting (Always Live for Telemetry) ---
    rpiInterface.update(); // Harvest high-speed vision data from UART1

    if (!cfgSimMode) {
        if (imuOK) imu.read(myIMU);
        if (tofOK) {
            if (tof.read(myToF)) {
                lastToFSuccessTime = currentTime; // Ping the watchdog: We are alive!
                
                // --- ToF Obstacle Debouncing Filter ---
                if (systemRunning && tof.evaluateObstacle(myToF, currentTime, lastAvoidanceEndTime)) {
                    Serial.print(">>> OBSTACLE DETECTED (<= ");
                    Serial.print(OBSTACLE_DISTANCE_THRESHOLD_MM);
                    Serial.println("mm)! Triggering Avoidance Maneuver...");
                        
                    status.changeState(SystemState::LINE_TRACING, SubStatus::TRACE_OBSTACLE_AVOIDANCE);
                    status.update(); // Flush the visual indicator immediately
                        
                    bool maneuverCompleted = driver.executeAvoidanceManeuver(cfgAvoidRight);
                        
                    if (!maneuverCompleted) {
                        // GP21 was pressed DURING the avoidance maneuver. 
                        // Ensure the system completely disarms.
                        systemRunning = false;
                        isPaused = false;
                        simPauseOffset = 0;
                        status.changeState(SystemState::IDLE_STANDBY, SubStatus::NONE_OK);
                    } else {
                        lastAvoidanceEndTime = millis(); // Start the cooldown period!
                        Serial.println(">>> AVOIDANCE COMPLETE: ToF Sensor ignoring targets for 2.5s to clear obstacle.");
                        lastToFSuccessTime = millis(); // Suppress the watchdog
                        status.changeState(SystemState::LINE_TRACING, SubStatus::TRACE_NORMAL);
                    }
                }
            } else if (currentTime - lastToFSuccessTime > 1000) {
                // WATCHDOG TRIPPED: 1 second passed with no fresh data!
                Serial.println(">>> WATCHDOG: ToF sensor unresponsive! Executing live auto-recovery...");
                myToF.count = 0; // Clear the stale 196mm data so it disappears from the monitor
                tofOK = false;
                
                resetToF(); // Yank the hardware pin to force a physical reboot
                if (tof.begin(Wire)) {
                    tofOK = true;
                    Serial.println(">>> WATCHDOG: ToF Auto-Recovery SUCCESS!");
                } else {
                    Serial.println(">>> WATCHDOG: ToF Auto-Recovery FAILED. Retrying in 1s...");
                }
                lastToFSuccessTime = currentTime; // Reset the timer to space out recovery attempts
            }
        }
    }

    // --- Active Logic Controller Path Routing ---
    if (systemRunning) {
        
        if (cfgSimMode) {
            // ===================================================
            // SANDBOX EXECUTION ROUTE (Bypasses Sensor Reading)
            // ===================================================
            sim.update(currentTime);
            
        } else {
            // ===================================================
            // REAL MAP RUN ROUTE (Active Hardware Interception)
            // ===================================================

            // Real track evaluation loops and Python Serial parsers interface here:
            // Example:
            // if (Serial.available()) { ... status.changeState(...) }
        }

        // --- Hardware Muscle Execution Handlers ---
        if (cfgMotorEnable) {
            if (status.getState() == SystemState::LINE_TRACING && status.getSubStatus() == SubStatus::TRACE_NORMAL) {
                // Calculate Vision/PID requested speeds
                MotorOutput pdOutput = lineProcessor.compute(status.getState(), status.getSubStatus());
                driver.executeTrajectory(status.getState(), status.getSubStatus(), pdOutput.leftSpeed, pdOutput.rightSpeed);
            } else {
                // Pre-programmed maneuver
                driver.executeTrajectory(status.getState(), status.getSubStatus()); 
            }
        } else {
            motors.stop(); // Safe static posture for desk/manual testing
        }
        
        ballHandler.processRescueTask(status.getSubStatus());                  // Payload actions handle sorting tasks

    } else {
        // Enforce safe static posture if process loop is deactivated
        motors.stop();
    }

    // --- Serial Stream Monitor Telemetry Output ---
    if (cfgEnablePrint && (currentTime - lastLogTime >= LOG_INTERVAL_MS)) {
        lastLogTime = currentTime;
        
        // Prevent USB stack from hanging if the computer isn't reading fast enough
        if (Serial && Serial.availableForWrite() > 64) {
            serialLogCounter++;
            
            Serial.print("Log #");
            Serial.print(serialLogCounter);
            Serial.print(" | MODE: [");
            if (cfgSimMode) Serial.print("SIMULATION");
            else Serial.print("LIVE_TRACK");
            Serial.print("] | Master State ID: ");
            Serial.print((int)status.getState());
            Serial.print(" | Sub-Action Focus Target: ");
            Serial.println((int)status.getSubStatus());

            if (!cfgSimMode) {
                // Output sensor matrices only during live execution tracks
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
                
                // Output Vision Telemetry
                Serial.print("   -> Vision | Error: "); Serial.print(rpiInterface.getPixelError());
                Serial.print(" | Green Code: "); Serial.println(rpiInterface.getGreenCode());
            }
        }
    }
    
    // Critical: Yield time to the RP2350 USB background stack to prevent crashes
    yield();
}