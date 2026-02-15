# Battery Management Firmware (Issue #46)

**Hardware:** RP2350 (Microcontroller) & BQ76920 (Battery Monitor)  
**Status:** In Development

## Overview
This firmware acts as the "safety brain" for the submarine's battery. It interfaces with the BQ76920 IC over I2C to monitor real-time Voltage and Current. 

**Key Principle:** "Don't Kill the Sub Instantly."
To protect the Jetson and other sensitive electronics, this firmware implements a strict escalation policy. It provides visual and audible warnings *before* cutting power, allowing the operator to surface or shut down safely.

## Warning & Kill Logic
The system progresses through 4 states based on battery health. Thresholds are defined at the top of `src/main.c` and can be easily adjusted.

| State | Condition (Configurable) | Behavior |
| :--- | :--- | :--- |
| **Normal** | Voltage > 15V | System Operational. |
| **Warn 1** | Voltage < 15V | **Red LED** turns ON. |
| **Warn 2** | Voltage < 14V | **Red LED** ON + **Buzzer** ON. |
| **Warn 3** | Voltage < 13V | **Red LED** Flashes + **Buzzer** ON. |
| **KILL** | Voltage < 10V OR Current > 2A | **MOSFET Disconnects Power.** |

## Hardware Pinout
*Consult with the PCB Designer (Issue #46 Assignee) to verify these pins.*

| Component | RP2350 Pin | Function |
| :--- | :--- | :--- |
| **I2C SDA** | GPIO 4 | Data Line to BQ76920 |
| **I2C SCL** | GPIO 5 | Clock Line to BQ76920 |
| **Red LED** | GPIO 14 | Warning Indicator |
| **Buzzer** | GPIO 15 | Audible Alarm |
| **MOSFET** | GPIO 16 | Main Power Cutoff (High = ON) |

## Telemetry & Graphing
The board outputs raw telemetry over USB Serial (baud rate 115200). You can view this data in the Arduino IDE Serial Plotter or any serial terminal.

**Data Format:**
```text
DATA, [Voltage_V], [Current_A]
