# Battery Management Firmware (Issue #46)

**Hardware:** RP2350 (Microcontroller) & BQ76920 (Battery Monitor)  
**Status:** In Development 🏗️  
**Developer:** Harsha Gundala Sureshbabu  
**Deadline:** End of Spring Semester  

## Overview
This firmware acts as the "safety brain" for the submarine's battery system. It interfaces with the **TI BQ76920** Li-ion monitor IC over **I2C** to monitor real-time Voltage and Current. 

### Key Principle: "Don't Kill the Sub Instantly."
To protect the Jetson and other sensitive electronics, the firmware implements a strict escalation policy. It provides visual and audible warnings before cutting power, allowing the operator time to surface or shut down safely rather than facing an abrupt power loss underwater.



---

## Warning & Kill Logic
The system transitions through states based on real-time battery health telemetry. 

| State | Condition (Configurable) | Behavior |
| :--- | :--- | :--- |
| **Normal** | Voltage > 15V | System Operational. |
| **Warn 1** | Voltage < 15V | 🔴 Red LED turns ON. |
| **Warn 2** | Voltage < 14V | 🔴 Red LED ON + 🔊 Buzzer ON. |
| **Warn 3** | Voltage < 13V | 🔴 Red LED Flashes + 🔊 Buzzer ON. |
| **CRITICAL/KILL** | Voltage < 10V OR Current > 2A | ⚡ MOSFET Disconnects Power. |

---

## Project Structure
```text
Battery-Management-System/
├── src/
│   ├── main.c               # Main monitoring loop 
│   ├── bq76920.c            # BQ76920 I2C driver implementation
│   ├── bq76920.h            # Register maps and bitmask definitions
│   ├── config.h             # User-configurable thresholds & pinouts
│   ├── warning_system.c     # LED/Buzzer state machine logic
│   └── warning_system.h     # Warning system header
├── CMakeLists.txt           # Pico SDK build configuration
├── pico_sdk_import.cmake    # RP2350 environment integration
└── README.md                # Project documentation
