#ifndef BQ76920_H
#define BQ76920_H

#include "hardware/i2c.h"
#include <stdint.h>
#include <stdbool.h>

/* --- Register Addresses (see datasheet Section 8.5) --- */

#define BQ_REG_SYS_STAT         0x00
#define BQ_REG_CELLBAL1         0x01
#define BQ_REG_SYS_CTRL1        0x04
#define BQ_REG_SYS_CTRL2        0x05
#define BQ_REG_PROTECT1         0x06
#define BQ_REG_PROTECT2         0x07
#define BQ_REG_PROTECT3         0x08
#define BQ_REG_OV_TRIP          0x09
#define BQ_REG_UV_TRIP          0x0A
#define BQ_REG_CC_CFG           0x0B
#define BQ_REG_VC1_HI           0x0C
#define BQ_REG_VC1_LO           0x0D
#define BQ_REG_VC2_HI           0x0E
#define BQ_REG_VC2_LO           0x0F
#define BQ_REG_VC3_HI           0x10
#define BQ_REG_VC3_LO           0x11
#define BQ_REG_VC4_HI           0x12
#define BQ_REG_VC4_LO           0x13
#define BQ_REG_VC5_HI           0x14
#define BQ_REG_VC5_LO           0x15
#define BQ_REG_BAT_HI           0x2A
#define BQ_REG_BAT_LO           0x2B
#define BQ_REG_CC_HI            0x32
#define BQ_REG_CC_LO            0x33
#define BQ_REG_ADCGAIN1         0x50
#define BQ_REG_ADCOFFSET        0x51
#define BQ_REG_ADCGAIN2         0x59

/* --- SYS_STAT fault flags --- */

#define BQ_STAT_OCD             (1 << 0)  /* overcurrent discharge */
#define BQ_STAT_SCD             (1 << 1)  /* short circuit */
#define BQ_STAT_OV              (1 << 2)  /* overvoltage */
#define BQ_STAT_UV              (1 << 3)  /* undervoltage */
#define BQ_STAT_OVRD_ALERT      (1 << 4)
#define BQ_STAT_DEVICE_XREADY   (1 << 5)  /* chip not ready yet */
#define BQ_STAT_CC_READY        (1 << 7)  /* coulomb counter has new data */

/* --- SYS_CTRL2 bits for FET and coulomb counter control --- */

#define BQ_CTRL2_CHG_ON         (1 << 0)
#define BQ_CTRL2_DSG_ON         (1 << 1)
#define BQ_CTRL2_CC_EN          (1 << 5)
#define BQ_CTRL2_CC_ONESHOT     (1 << 6)

/* all values are in mV and mA to keep things simple */
typedef struct {
    int32_t  voltage_mv;
    int32_t  current_ma;
    uint8_t  fault_flags;
    bool     valid;             /* set to false if i2c read failed */
} bq_sensor_data_t;

/* gain is in µV/LSB, offset in mV — read from chip during init */
typedef struct {
    uint16_t gain_uv;
    int8_t   offset_mv;
} bq_calibration_t;

/* sets up i2c, reads calibration from chip, turns on coulomb counter */
bool bq76920_init(i2c_inst_t *i2c_port, uint sda_pin, uint scl_pin);

/* grabs voltage, current, and faults in one shot — check .valid before using */
bq_sensor_data_t bq76920_read_sensors(void);

/* returns pack voltage in mV, or -1 if i2c fails */
int32_t bq76920_read_voltage(void);

/* positive means discharging, returns 0 on error */
int32_t bq76920_read_current(void);

/* use BQ_STAT_* masks to check which faults are active */
uint8_t bq76920_read_faults(void);

/* write 1s to the bits you want to clear */
void bq76920_clear_faults(uint8_t flags);

#endif