#ifndef CONFIG_H
#define CONFIG_H

// =======================================================
// SYSTEM CONFIGURATION & TUNING PARAMETERS
// =======================================================

// --- Tuning Parameters ---
constexpr int OBSTACLE_DISTANCE_THRESHOLD_MM = 60;  // Distance in mm to trigger obstacle avoidance
constexpr int OBSTACLE_DEBOUNCE_COUNT        = 3;   // Number of consecutive detections to confirm obstacle
constexpr int TOF_MIN_VALID_DIST_MM          = 10;  // Ignore readings below this (e.g., 0 error)
constexpr int TOF_MAX_VALID_DIST_MM          = 500; // Ignore readings above this (e.g., 65535 error)
constexpr int BASE_TRACKING_SPEED            = 60;  // Normal line tracing speed (0-255)

// --- Vision Control / PID Parameters ---
constexpr float DEFAULT_LINE_KP              = 0.2f; // Proportional tuning
constexpr float DEFAULT_LINE_KD              = 0.1f; // Derivative tuning
constexpr int   LINE_MAX_PIXEL_OFFSET        = 320;  // Assume 640px wide camera, so 320 is edge
constexpr int   GREEN_TURN_BIAS              = 140;  // Artificial offset injected for a green square turn
constexpr int   GREEN_DEBOUNCE_FRAMES        = 3;    // How many frames in a row the Pi must see green

// --- Obstacle Avoidance Arc Parameters ---
constexpr int OBSTACLE_AVOID_ARC_OUTER_SPEED = 120;  // Outer wheel speed during circular sweep
constexpr int OBSTACLE_AVOID_ARC_INNER_SPEED = 20;   // Inner wheel speed during circular sweep
constexpr float OBSTACLE_AVOID_ARC_FINISH_DEG = 10.0; // Degrees tolerance to finish arc trajectory

// --- Dynamic Braking IMU Turn Parameters ---
constexpr int   OBSTACLE_AVOID_TURN_BASE_SPEED = 120;  // Base maximum cruising speed for IMU pivots
constexpr float OBSTACLE_AVOID_TURN_BRAKE_DEG  = 2.0;  // Degrees remaining to trigger active brake lock
constexpr float OBSTACLE_AVOID_TURN_BRAKE_PCT  = 0.2;  // Active brake reverse speed multiplier (0.2 = 20%)
constexpr int   OBSTACLE_AVOID_TURN_BRAKE_MS   = 20;   // Time (ms) to fire the reverse active brake

// --- Default Runtime Settings ---
// These set the initial boot state of the robot before serial commands override them
constexpr bool DEFAULT_SIM_MODE             = false; // true = virtual timeline sandbox; false = Live track
constexpr bool DEFAULT_REQUIRE_START_BUTTON = true;  // true = Wait in Standby for start button
constexpr bool DEFAULT_ENABLE_PRINT         = true;  // true = Output telemetry data
constexpr bool DEFAULT_MOTOR_ENABLE         = true;  // true = Hardware motors active
constexpr bool DEFAULT_AVOID_RIGHT          = true;  // true = Dodge Right; false = Dodge Left

// --- Hardware Pins ---
constexpr int START_PIN = 20;   // GP20 Start Button
constexpr int STOP_PIN  = 21;   // GP21 Stop Button
constexpr int HEART_PIN = 0;    // GP0 Heartbeat onboard LED
constexpr int XSHUT_PIN = 18;   // GP18 Hardware Shutdown pin for VL53L4CX

// --- I2C Bus Pins ---
constexpr int TOF_SDA = 16;
constexpr int TOF_SCL = 17;
constexpr int IMU_SDA = 26;
constexpr int IMU_SCL = 27;

#endif