#include "Serial_Control.h"

Serial_Control::Serial_Control(Camera_Mount& camMount, Ball_Handler& ballHandler, Motor_Control& motors, bool& simMode, bool& requireStartButton, bool& enablePrint, bool& motorEnable)
    : _camMount(camMount), _ballHandler(ballHandler), _motors(motors),
      _simMode(simMode), _requireStartButton(requireStartButton), _enablePrint(enablePrint), _motorEnable(motorEnable) {}

void Serial_Control::begin() {
    Serial.println("Serial_Control: Initialized and listening for commands.");
}

void Serial_Control::update() {
    if (Serial.available() > 0) {
        String debugCmd = Serial.readStringUntil('\n');
        processCommand(debugCmd);
    }
}

bool Serial_Control::isVirtualStartPressed() {
    if (_virtualStartTrigger) {
        _virtualStartTrigger = false; // Clear the flag after reading (like a physical button release)
        return true;
    }
    return false;
}

bool Serial_Control::isVirtualStopPressed() {
    if (_virtualStopTrigger) {
        _virtualStopTrigger = false;
        return true;
    }
    return false;
}

void Serial_Control::processCommand(String debugCmd) {
    debugCmd.trim(); debugCmd.toUpperCase();
    
    // Echo the raw intercepted command to the Serial Monitor
    if (debugCmd.length() > 0) {
        Serial.print(">>> Raw Command Received: [");
        Serial.print(debugCmd);
        Serial.println("]");
        
        // Parse space-separated commands (e.g. "CLAW:80 ARM:70 SIM:ON")
        while (debugCmd.length() > 0) {
            int spaceIndex = debugCmd.indexOf(' ');
            String token;
            
            if (spaceIndex != -1) {
                token = debugCmd.substring(0, spaceIndex);
                debugCmd = debugCmd.substring(spaceIndex + 1);
                debugCmd.trim(); // Clean up multiple spaces
            } else {
                token = debugCmd;
                debugCmd = "";
            }

            int colonIndex = token.indexOf(':');
            if (colonIndex != -1) {
                String key = token.substring(0, colonIndex);
                String value = token.substring(colonIndex + 1);
                
                Serial.print(">>> Parsed -> Key: ["); Serial.print(key);
                Serial.print("] Value: ["); Serial.print(value); Serial.println("]");

                // Pass parsing tokens downstream to respective physical structural owners
                if (key == "TILT" || key == "PAN") {
                    Serial.println(">>> Routing command to Camera_Mount...");
                    _camMount.handleDebugSerial(key, value);
                } else if (key == "ARM" || key == "CLAW") {
                    Serial.println(">>> Routing command to Ball_Handler...");
                    _ballHandler.handleDebugSerial(key, value);
                } else if (key == "SIM") {
                    if (value == "ON") {
                        _simMode = true;
                        Serial.println(">>> Simulation Mode: ON");
                    } else if (value == "OFF") {
                        _simMode = false;
                        Serial.println(">>> Simulation Mode: OFF");
                    } else {
                        Serial.println(">>> Error: Invalid SIM argument (Expected ON or OFF).");
                    }
                } else if (key == "RUN") {
                    if (value == "ON") {
                        _virtualStartTrigger = true;
                        Serial.println(">>> Virtual Command: STARTING TRACK ROUTINE");
                    } else if (value == "OFF") {
                        _virtualStopTrigger = true;
                        Serial.println(">>> Virtual Command: HALTING TRACK ROUTINE");
                    } else {
                        Serial.println(">>> Error: Invalid RUN argument (Expected ON or OFF).");
                    }
                } else if (key == "STARTBTN") {
                    if (value == "ON") {
                        _requireStartButton = true;
                        Serial.println(">>> Start Button Requirement: ON");
                    } else if (value == "OFF") {
                        _requireStartButton = false;
                        Serial.println(">>> Start Button Requirement: OFF");
                    } else {
                        Serial.println(">>> Error: Invalid STARTBTN argument (Expected ON or OFF).");
                    }
                } else if (key == "PRINT") {
                    if (value == "ON") {
                        _enablePrint = true;
                        Serial.println(">>> Serial Print: ON");
                    } else if (value == "OFF") {
                        _enablePrint = false;
                        Serial.println(">>> Serial Print: OFF");
                    } else {
                        Serial.println(">>> Error: Invalid PRINT argument (Expected ON or OFF).");
                    }
                } else if (key == "MOTOR") {
                    if (value == "ON") {
                        _motorEnable = true;
                        Serial.println(">>> Motor Output: ON");
                    } else if (value == "OFF") {
                        _motorEnable = false;
                        Serial.println(">>> Motor Output: OFF (Manual placement mode)");
                    } else {
                        Serial.println(">>> Error: Invalid MOTOR argument (Expected ON or OFF).");
                    }
                } else if (key == "TESTDRIVE") {
                    // Parse "L_SPEED,R_SPEED,TIME_MS" -> e.g. TESTDRIVE:50,50,5000
                    int firstComma = value.indexOf(',');
                    int secondComma = value.indexOf(',', firstComma + 1);
                    if (firstComma != -1 && secondComma != -1) {
                        int speedL = value.substring(0, firstComma).toInt();
                        int speedR = value.substring(firstComma + 1, secondComma).toInt();
                        int duration = value.substring(secondComma + 1).toInt();
                        
                        Serial.print(">>> CALIBRATION RUN: L: "); Serial.print(speedL);
                        Serial.print(" | R: "); Serial.print(speedR);
                        Serial.print(" | For: "); Serial.print(duration); Serial.println("ms");
                        
                        _motors.setSpeed(speedL, speedR, speedL, speedR);
                        delay(duration); // Blocks execution precisely for the test duration
                        _motors.stop();
                        
                        _motorEnable = false; // Keep the robot still for taking measurements
                        Serial.println(">>> CALIBRATION RUN COMPLETE. Motor Output safely DISABLED.");
                        
                    } else {
                        Serial.println(">>> Error: TESTDRIVE requires L_SPEED,R_SPEED,TIME_MS (e.g. TESTDRIVE:50,50,5000)");
                    }
                } else {
                    Serial.println(">>> Error: Unknown module target (Expected RUN, TILT, PAN, ARM, CLAW, SIM, STARTBTN, PRINT, MOTOR, or TESTDRIVE).");
                }
            } else if (token.length() > 0) {
                Serial.print(">>> Error: Invalid command format in [");
                Serial.print(token);
                Serial.println("]. Expected KEY:VALUE.");
            }
        }
    }
}