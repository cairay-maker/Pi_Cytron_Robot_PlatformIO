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