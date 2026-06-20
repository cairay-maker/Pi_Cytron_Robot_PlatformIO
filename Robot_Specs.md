# Team Leli - Robot Specifications & Kinematics

## 1. Physical Dimensions
| Dimension | Measurement |
| :--- | :--- |
| **Front Axle to Back Axle** | 112 mm |
| **Left to Right Center Tire** | 97 mm |
| **Tire Diameter** | 43 mm |
| **Tire Width** | 17 mm |

---

## 2. Obstacle Avoidance Strategy (Kinematics)
To successfully route around an obstacle of an arbitrary size (e.g., a 10cm wide bottle), the robot needs to know its own physical travel capabilities. We achieve this by measuring three fundamental movement scenarios at a baseline speed (e.g., Speed `50`).

By gathering this data, you can instruct the robot to perform precise geometric paths. 
**Pre-Programmed Avoidance Path (Configurable Left/Right):**
1. **Validation:** ToF sensor detects an obstacle at `<= 50mm` (Filtering out `0` and requiring 3 consecutive valid readings to avoid false positives).
2. **Check Avoidance Direction:** Evaluate pre-programmed competition flag (e.g., `avoid_left = true`).
3. **Phase 1 (Pivot Turn):** 
   - Right Avoidance: `TESTDRIVE:50,-50,3000` (Pivot Right)
4. **Phase 2 (Arc Sweep):** 
   - Right Avoidance: `TESTDRIVE:30,90,10000` (Arc Left around obstacle)
5. **Phase 3:** Main line tracing sensors detect the line and robot automatically resumes `LINE_TRACING`.

---

## 3. Calibration Run Logs
Use the Serial Monitor to run these exact diagnostic timed tests. 
*Command format:* `TESTDRIVE:LEFT_SPEED,RIGHT_SPEED,TIME_MS`

### Scenario A: Linear Velocity
**Purpose:** Determine how many centimeters the robot travels per second at a base speed.
**Commands (Copy & Paste):**
`TESTDRIVE:50,50,10000` *(Drive straight at speed 50 for 10 seconds)*
`TESTDRIVE:100,100,10000` *(Drive straight at speed 100 for 10 seconds)*

| Battery Level | Speed | Time (ms) | Distance Traveled (cm) | Result (cm/sec) |
| :--- | :--- | :--- | :--- | :--- |
| 100% (Full) | 50 | 10000 | 45.0 | 4.5 |
| 100% (Full) | 100 | 10000 | 92.0 | 9.2 |

### Scenario B: Pivot Turn Velocity
**Purpose:** Determine how many degrees the robot rotates per second when turning in place.
**Command (Copy & Paste):** 
`TESTDRIVE:-50,50,11000` *(Pivot in place at speed 50 for 11 seconds)*
`TESTDRIVE:50,-50,6000` *(Point 90 degree turn)*

| Battery Level | Speed | Time (ms) | Degrees Rotated | Result (deg/sec) |
| :--- | :--- | :--- | :--- | :--- |
| 100% (Full) | L:-50, R:50 | 11000 | 400 | 36.36 |
| 100% (Full) | L:50, R:-50 | 6000 | 90 | 15.0 |

*Note:* To successfully extract a 90-degree turn from the 400-degree rotation result, use a timed block of **2475ms** `(11000 * (90 / 400))`.

### Scenario C: Arc Turn Radius
**Purpose:** Determine the radius of a curve when inner and outer wheels spin at different speeds.
**Command (Copy & Paste):** 
`TESTDRIVE:20,100,10000` *(Sweep an arc to navigate around an obstacle)*

| Battery Level | Speed | Time (ms) | Path Diameter (cm) | Sweeping Radius (cm) |
| :--- | :--- | :--- | :--- | :--- |
| 100% (Full) | L:20, R:100 | 10000 | 15.0 | 7.5 |

*Note: This curve trajectory successfully navigates around a standard 10cm-wide bottle obstacle.*

---

## 4. Raspberry Pi OpenCV Vision Integration
The Cytron RP2350 serves as the motor and hardware controller, while the Raspberry Pi acts as the "Vision Brain" using OpenCV. They communicate serially over a high-speed hardware UART.

### Hardware Interface Map
*   **Protocol:** Serial UART (115200 baud)
*   **RPi TX (Pin 8 / GPIO 14)** → **Cytron RX (GP5 / UART1 RX)**
*   **RPi RX (Pin 10 / GPIO 15)** → **Cytron TX (GP4 / UART1 TX)**
*   **Common Ground:** Mandatory wire between RPi Pin 6 and Cytron GND.

*(Note: The Cytron RX pin has its internal `INPUT_PULLUP` enabled in software to prevent floating electrical noise from locking up the processor when the Pi is disconnected).*

### The OpenCV Vision Contract
The Python script on the Raspberry Pi must send data in the following format exactly 30 times per second:
`V:<pixel_error>,<green_code>\n`

*   **`<pixel_error>`**: An integer from roughly `-320` to `320`. 
    *   `0` = Robot perfectly centered on line.
    *   Positive Number = Line is to the right.
    *   Negative Number = Line is to the left.
*   **`<green_code>`**: 
    *   `0` = No green square detected.
    *   `1` = Left green square detected.
    *   `2` = Right green square detected.
    *   `3` = U-Turn.
    
**Crucial OpenCV Strategy (Region of Interest):** To prevent the robot from reacting to upcoming corners too early, the OpenCV script must crop the camera frame to a narrow "letterbox" (e.g., the bottom 40 pixels of a 320x180 frame) so it only evaluates the line directly in front of the wheels.

### Proportional-Derivative (PD) Tuning
The Cytron calculates the required motor speeds using a PD controller (`Line_Processor.cpp`).
*   `DEFAULT_LINE_KP = 0.2` (Proportional: How hard it steers)
*   `DEFAULT_LINE_KD = 0.1` (Derivative: How hard it resists sudden changes)
*   `BASE_TRACKING_SPEED = 60` (Cruising speed)
*   `GREEN_TURN_BIAS = 140` (Artificial error injected to force a tight 90-degree pivot without stopping)

### Live Desktop Tuning Commands
You can tune the robot's vision math via the PlatformIO Serial Monitor (USB-C) without recompiling, bypassing the Pi entirely!
*   **`RUN:ON`** - Starts the main execution loop.
*   **`MOTOR:OFF`** - Keeps wheels safe on the desk while allowing math output.
*   **`MOCK_V:60,2`** - Injects a fake Vision packet (e.g., Error 60, Green Right) to test the PD math.
*   **`PID:0.25,0.15,60`** - Updates `Kp`, `Kd`, and `BaseSpeed` on the fly.