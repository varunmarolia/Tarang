/*!
 * @file  sht4x.c
 * @author Varun Marolia
 * @brief This file implements the low level function bodies designed 
 *       specifically for a Sensirion Temperature and humidity sensor sht4x series.
 * @copyright Copyright (c) 2025 Varun Marolia
 *   MIT License
 *   Permission is hereby granted, free of charge, to any person obtaining a copy
 *   of this software and associated documentation files (the "Software"), to deal
 *   in the Software without restriction, including without limitation the rights
 *   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *   copies of the Software, and to permit persons to whom the Software is
 *   furnished to do so, subject to the following conditions:**
 *
 *   The above copyright notice and this permission notice shall be included in all
 *   copies or substantial portions of the Software.
 *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *   SOFTWARE.
 * 
 */

#include "board.h"
#include "timer.h"          /* included for startup and measurement sampling timer */
#include "sht4x.h"

#define DEBUG_SHT4X  0 /**< This macro enables/disables the debug printf for this file */

#if DEBUG_SHT4X
#include <stdio.h>
#undef PRINTF
#define PRINTF(...) printf(__VA_ARGS__)
#else   /* DEBUG_SHT4X */
#define PRINTF(...)                       /**< Replace printf with nothing */
#endif  /* DEBUG_SHT4X */

static const crc8_cfg_t sht4x_crc_cfg = {
  .polynomial = SHT4X_CRC8_POLYNOMIAL,
  .intial_remainder = SHT4X_CRC8_INITIAL_REMAINDER,
  .final_xor_value = SHT4X_CRC8_FINAL_XOR_VALUE
};

typedef enum {
  SHT4X_SINGLE_MEASUREMENT_HIGH_REP_CLKSTRETCH_DISABLE,
  SHT4X_SINGLE_MEASUREMENT_MEDIUM_REP_CLKSTRETCH_DISABLE,
  SHT4X_SINGLE_MEASUREMENT_LOW_REP_CLKSTRETCH_DISABLE,
  SHT4X_READ_SERIAL_NUMBER,
  SHT4X_SOFT_RESET,
  SHT4X_HEATER_ENABLE_200MW_1SEC,
  SHT4X_HEATER_ENABLE_200MW_0_1SEC,
  SHT4X_HEATER_ENABLE_110MW_1SEC,
  SHT4X_HEATER_ENABLE_110MW_0_1SEC,
  SHT4X_HEATER_ENABLE_20MW_1SEC,
  SHT4X_HEATER_ENABLE_20MW_0_1SEC
} sht4x_cmd;

static const sensirion_cmd_t sht4x_commands[] = {
/* cmd, read/write data length, response time in milliseconds */
  {0xFD, 6, 9},     /* Single shot measurement, High repeatability, Clock stretching Disabled */
  {0xF6, 6, 5},     /* Single shot measurement, Medium repeatability, Clock stretching Disabled */
  {0xE0, 6, 2},     /* Single shot measurement, Low repeatability, Clock stretching Disabled */
  {0x89, 6, 2},     /* read chip serial number */
  {0x94, 0, 2},     /* Soft reset command. The system must be in idle state (not performing measurement) before issuing. */
  {0x39, 6, 1},     /* Activate heater 200mW for 1s, including a high precision measurement before deactivation */
  {0x32, 6, 1},     /* Activate heater 200mW for 0.1s, including a high precision measurement before deactivation */
  {0x2F, 6, 1},     /* Activate heater 110mW for 1s, including a high precision measurement before deactivation */
  {0x24, 6, 1},     /* Activate heater 110mW for 0.1s, including a high precision measurement before deactivation */
  {0x1E, 6, 1},     /* Activate heater 20mW for 1s, including a high precision measurement before deactivation */
  {0x15, 6, 1},     /* Activate heater 20mW for 0.1s, including a high precision measurement before deactivation */
};
static sensirion_device_t sht4x_device_config = {
  .cmd_set = sht4x_commands,
  .crc_config = &sht4x_crc_cfg,
  .dev = NULL,
  .cmd_num = sizeof(sht4x_commands) / sizeof(sensirion_cmd_t),
  .cmd_bytes = 1
};
/*---------------------------------------------------------------------------*/
uint8_t
sht4x_init(sht4x_t *sht)
{
  uint8_t sht4x_status = BUS_OK;
  uint16_t sht4x_serial[2];
  if(sht != NULL && sht->sht4x_dev != NULL) {
#ifdef SHT4X_POWER_ON
  /* turn ON power - must include 2 ms startup delay */
    SHT4X_POWER_ON();
#endif  /* SHT4X_POWER_ON() */
    sht4x_device_config.dev = sht->sht4x_dev;
    /* read status register for probing purposes*/
    sht4x_status = sensirion_get(&sht4x_device_config, SHT4X_READ_SERIAL_NUMBER, sht4x_serial, 2);
    if(sht4x_status != BUS_OK) {
      PRINTF("SHT4X failed to read serial number!!!\n");
    } else {
      sht->serial_number = sht4x_serial[0]; /* MSB */
      sht->serial_number = sht->serial_number << 16;
      sht->serial_number |= sht4x_serial[1]; /* LSB */
      PRINTF("SHT4X serial number:0x%04X\n", sht->serial_number);
    }
  }
  sht4x_status = BUS_INVALID;
  return sht4x_status;
}
/*---------------------------------------------------------------------------*/
uint8_t
sht4x_take_single_measurement(sht4x_t *sht)
{
  uint8_t sht4x_status;
  uint16_t sht4x_data[2];
  int32_t rh_value;
  if(sht !=NULL && sht->sht4x_dev != NULL) {
    sht4x_device_config.dev = sht->sht4x_dev;
    /* It is assumed here that the sensor is not configured in continuous mode */
    /* Configure measurement type here. Single shot measurement with high repeatability and clock stretching disabled */
    sht4x_status = sensirion_get(&sht4x_device_config, SHT4X_SINGLE_MEASUREMENT_HIGH_REP_CLKSTRETCH_DISABLE, sht4x_data, 2);
    if(sht4x_status != BUS_OK) {
      PRINTF("SHT4X failed to read data!!!\n");
      return sht4x_status;
    }
    /**
      * convert temperature into millikelvin from raw reading
      * T[C] = -45 + 175 * (measurement value) / 65535
      * T[mK] = T[mC] + 273150
      * T[mK] = 228150 + (267 * (measurement value)) / 100
    */
    sht->last_temp_mk = (sht4x_data[0] * 267) / 100 + 228150;
    /**
     * convert RH into %
     *  RH[%] = -6 + 125 * (Measurement value) / 65535
     * */
    rh_value = (int32_t)(((sht4x_data[1] * 125 * 10000ULL) / 65535) - 60000); /* 10,000 times scaled to get ppm */
    if(rh_value < 0) {
      sht->last_rh_ppm = 0;
    } else if(rh_value > 1000000UL) {
      sht->last_rh_ppm = 1000000;
    } else {
      sht->last_rh_ppm = (uint32_t)rh_value;
    }
  }
  return BUS_OK;
}
/*---------------------------------------------------------------------------*/
uint32_t
sht4x_get_serial_id(sht4x_t *sht)
{
  if(sht !=NULL) {
   return sht->serial_number;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
