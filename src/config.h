/**
 * @file config.h
 * @brief BMS configuration — thresholds, pins, and system parameters.
 *
 * Hardware: RP2350 + TI BQ76920
 * Project:  SubjuGator 9 BMS (Issue #46)
 *
 * Units: voltage in mV, current in mA, time in ms.
 * This is the ONLY file you need to edit to change BMS behavior.
 */

#ifndef CONFIG_H
#define CONFIG_H

/* ======================== WARNING THRESHOLDS ============================== */

#define WARN1_THRESHOLD_MV      15000   /* 15.0V — Red LED ON */
#define WARN2_THRESHOLD_MV      14000   /* 14.0V — Red LED + Buzzer ON */
#define WARN3_THRESHOLD_MV      13000   /* 13.0V — Red LED FLASH + Buzzer ON */

/* ======================== KILL CONDITIONS ================================= */

#define KILL_VOLTAGE_MV         10000   /* 10.0V — disconnect battery */
#define KILL_CURRENT_MA         2000    /* 2.0A  — overcurrent disconnect */

/* ======================== STABILITY SETTINGS ============================== */

/** Recovery band: must exceed threshold + this value to clear a warning. */
#define HYSTERESIS_MV           200     /* 0.2V */

/** Consecutive violations before state change. 5 × 100ms = 500ms debounce. */
#define DEBOUNCE_COUNT          5

/* ======================== TIMING ========================================== */

#define LOOP_INTERVAL_MS        100     /* Main loop period (10 Hz) */
#define LED_FLASH_PERIOD_MS     500     /* WARN3 LED toggle half-period */
#define LED_FLASH_FAST_MS       150     /* KILL LED toggle half-period */

/* ======================== I2C CONFIGURATION =============================== */

#define BMS_I2C_PORT            i2c0
#define BMS_I2C_FREQ_HZ        100000  /* 100kHz */
#define SDA_PIN                 4       /* I2C data  — verify with schematic */
#define SCL_PIN                 5       /* I2C clock — verify with schematic */
#define BQ76920_I2C_ADDR        0x08    /* 7-bit default address */

/* ======================== GPIO PIN ASSIGNMENTS ============================ */
/*  Coordinate with Issue #46 hardware assignee for final values.            */

#define LED_PIN                 16      /* Red warning LED */
#define BUZZER_PIN              17      /* Piezo buzzer */
#define MOSFET_PIN              18      /* Gate control (HIGH = connected) */

/* ======================== OPTIONAL FEATURES =============================== */

/** 1 = CSV logging over USB serial (timestamp, voltage, current, state). */
#define DATA_LOGGING_ENABLED    1

/* ======================== BQ76920 CALIBRATION ============================= */

/** Shunt resistor (mΩ) for current measurement — must match PCB. */
#define SHUNT_RESISTOR_MOHM     10      /* 10 mΩ */

#endif /* CONFIG_H */