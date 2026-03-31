#include "bq76920.h"
#include "config.h"
#include "pico/stdlib.h"
#include <string.h>

/* driver state — shared across all functions in this file */
static i2c_inst_t    *s_i2c       = NULL;
static bq_calibration_t s_cal     = {0};
static bool            s_initialized = false;

/* crc8 with polynomial 0x07 — the bq76920 won't talk to you without this */
static uint8_t crc8_compute(const uint8_t *data, uint8_t len) {
    uint8_t crc = 0x00;
    for (uint8_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t bit = 0; bit < 8; bit++) {
            crc = (crc & 0x80) ? ((crc << 1) ^ 0x07) : (crc << 1);
        }
    }
    return crc;
}

/* reads one register and checks the crc the chip sends back.
   returns false if i2c fails or crc doesn't match */
static bool bq_read_reg(uint8_t reg, uint8_t *value) {
    uint8_t buf[2];

    int ret = i2c_write_blocking(s_i2c, BQ76920_I2C_ADDR, &reg, 1, true);
    if (ret < 0) return false;

    ret = i2c_read_blocking(s_i2c, BQ76920_I2C_ADDR, buf, 2, false);
    if (ret < 0) return false;

    /* crc covers the entire i2c transaction: address bytes + register + data */
    uint8_t crc_input[4] = {
        (uint8_t)(BQ76920_I2C_ADDR << 1),
        reg,
        (uint8_t)((BQ76920_I2C_ADDR << 1) | 0x01),
        buf[0]
    };
    uint8_t expected_crc = crc8_compute(crc_input, 4);

    if (buf[1] != expected_crc) return false;

    *value = buf[0];
    return true;
}

/* writes one register with crc appended — chip ignores the write if crc is wrong */
static bool bq_write_reg(uint8_t reg, uint8_t value) {
    uint8_t crc_input[3] = {
        (uint8_t)(BQ76920_I2C_ADDR << 1),
        reg,
        value
    };
    uint8_t crc = crc8_compute(crc_input, 3);

    uint8_t buf[3] = { reg, value, crc };
    int ret = i2c_write_blocking(s_i2c, BQ76920_I2C_ADDR, buf, 3, false);
    return (ret >= 0);
}

/* reads two consecutive registers as a 16-bit value (hi byte first) */
static int32_t bq_read_16bit(uint8_t reg_hi) {
    uint8_t hi, lo;
    if (!bq_read_reg(reg_hi, &hi))     return -1;
    if (!bq_read_reg(reg_hi + 1, &lo)) return -1;
    return (int32_t)((hi << 8) | lo);
}

/* pulls adc gain and offset from factory-programmed otp.
   gain is awkwardly split across two registers — thanks TI */
static bool bq_read_calibration(void) {
    uint8_t gain1, gain2, offset;

    if (!bq_read_reg(BQ_REG_ADCGAIN1, &gain1))  return false;
    if (!bq_read_reg(BQ_REG_ADCGAIN2, &gain2))  return false;
    if (!bq_read_reg(BQ_REG_ADCOFFSET, &offset)) return false;

    uint8_t gain_bits = ((gain1 & 0x18) << 1) | ((gain2 & 0xE0) >> 5);
    s_cal.gain_uv  = 365 + (uint16_t)gain_bits;
    s_cal.offset_mv = (int8_t)offset;

    return true;
}

/* sets up i2c, clears startup faults, reads calibration, and
   turns on the coulomb counter so we can measure current */
bool bq76920_init(i2c_inst_t *i2c_port, uint sda_pin, uint scl_pin) {
    s_i2c = i2c_port;

    i2c_init(s_i2c, BMS_I2C_FREQ_HZ);
    gpio_set_function(sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(scl_pin, GPIO_FUNC_I2C);
    gpio_pull_up(sda_pin);
    gpio_pull_up(scl_pin);

    /* give the chip a moment after power-on */
    sleep_ms(50);

    uint8_t status;
    if (!bq_read_reg(BQ_REG_SYS_STAT, &status)) return false;
    if (status & BQ_STAT_DEVICE_XREADY) {
        bq_write_reg(BQ_REG_SYS_STAT, BQ_STAT_DEVICE_XREADY);
        sleep_ms(10);
    }

    if (!bq_read_calibration()) return false;

    uint8_t ctrl2;
    if (!bq_read_reg(BQ_REG_SYS_CTRL2, &ctrl2)) return false;
    ctrl2 |= BQ_CTRL2_CC_EN | BQ_CTRL2_DSG_ON;
    if (!bq_write_reg(BQ_REG_SYS_CTRL2, ctrl2)) return false;

    /* 0x19 is the magic value TI recommends in the datasheet */
    if (!bq_write_reg(BQ_REG_CC_CFG, 0x19)) return false;

    s_initialized = true;
    return true;
}

/* pack voltage in mV — the x4 is because of the internal voltage divider */
int32_t bq76920_read_voltage(void) {
    if (!s_initialized) return -1;

    int32_t raw = bq_read_16bit(BQ_REG_BAT_HI);
    if (raw < 0) return -1;

    int32_t voltage_mv = (4 * (int32_t)s_cal.gain_uv * raw) / 1000
                         + (4 * (int32_t)s_cal.offset_mv);

    return voltage_mv;
}

/* current in mA from coulomb counter — positive means discharging.
   returns 0 if no new reading is available yet */
int32_t bq76920_read_current(void) {
    if (!s_initialized) return 0;

    uint8_t status;
    if (!bq_read_reg(BQ_REG_SYS_STAT, &status)) return 0;
    if (!(status & BQ_STAT_CC_READY)) return 0;

    bq_write_reg(BQ_REG_SYS_STAT, BQ_STAT_CC_READY);

    int32_t raw = bq_read_16bit(BQ_REG_CC_HI);
    if (raw < 0) return 0;

    /* raw is two's complement — 8.44µV per LSB divided by shunt resistance */
    int16_t raw_signed = (int16_t)raw;
    int32_t current_ma = ((int32_t)raw_signed * 8440) / (SHUNT_RESISTOR_MOHM * 1000);

    return current_ma;
}

uint8_t bq76920_read_faults(void) {
    uint8_t status = 0;
    bq_read_reg(BQ_REG_SYS_STAT, &status);
    return status;
}

/* write 1s to the fault bits you want to clear */
void bq76920_clear_faults(uint8_t flags) {
    bq_write_reg(BQ_REG_SYS_STAT, flags);
}

/* one-stop read for everything — check .valid before trusting the data */
bq_sensor_data_t bq76920_read_sensors(void) {
    bq_sensor_data_t data = {0};

    data.voltage_mv  = bq76920_read_voltage();
    data.current_ma  = bq76920_read_current();
    data.fault_flags = bq76920_read_faults();

    data.valid = (data.voltage_mv >= 0);

    return data;
}