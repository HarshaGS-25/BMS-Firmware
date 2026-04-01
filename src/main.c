#include <stdio.h>
#include "pico/stdlib.h"
#include "bq76920.h"
#include "warning_system.h"
#include "config.h"

int main() {
    // 1. Initialize Standard IO (allows printf over USB)
    stdio_init_all();
    
    // Give the user a moment to open a serial monitor
    sleep_ms(2000);
    printf("SubjuGator 9 BMS Initializing...\n");

    // 2. Initialize the BQ76920 Battery Driver
    // Uses pins and frequency defined in config.h
    if (!bq76920_init(BMS_I2C_PORT, SDA_PIN, SCL_PIN)) {
        printf("ERROR: BQ76920 not found on I2C bus!\n");
        // In a real sub, we might want to signal a hardware fault here
    } else {
        printf("BQ76920 Driver: OK\n");
    }

    // 3. Initialize the Warning System (LEDs, Buzzer, MOSFET)
    warning_system_init(LED_PIN, BUZZER_PIN, MOSFET_PIN);
    printf("Warning System: OK\n");

    // 4. Main Monitoring Loop
    while (true) {
        // Read raw data from the battery chip
        bq_sensor_data_t current_stats = bq76920_read_sensors();

        // Feed that data into the safety state machine
        warning_system_update(&current_stats);

        // Optional: Log data to USB serial for debugging
        #if DATA_LOGGING_ENABLED
        if (current_stats.valid) {
            printf("[%s] V: %dmV | I: %dmA | State: %s\n", 
                   "BMS",
                   current_stats.voltage_mv, 
                   current_stats.current_ma, 
                   warning_system_state_name());
        } else {
            printf("WARNING: Sensor Data Invalid (I2C Failure?)\n");
        }
        #endif

        // Wait for the interval defined in config.h (usually 100ms)
        sleep_ms(LOOP_INTERVAL_MS);
    }

    return 0;
}