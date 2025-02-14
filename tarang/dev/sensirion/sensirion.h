/*!
*\file  sensirion.h
*\brief This file hold the macros and low level function prototypes 
*       designed specifically for sensirion sensors in I2C mode.
*/
#ifndef SENSIRION_H_
#define SENSIRION_H_
#include <stdint.h>
#include "crc8.h"
#include "serial-dev.h"

#define SENSIRION_MAX_SET_PARAM_LENGTH  6   /* Maximum parameter length when writing command */
#define SENSIRION_MAX_GET_PARAM_LENGTH  60  /* Maximum parameter length when reading command */

#define SENSIRION_CMD_ATTRIBUTES        3   /* Total number of different attributes defined in spgc3_command array*/
#define SENSIRION_CMD_COLUMN            0   /* array column 0 defines different commands */
#define SENSIRION_PARAM_LENGTH_COLUMN   1   /* array column 1 defines parameter length*/
#define SENSIRION_DURATION_COLUMN       2   /* array column 2 defines measurement duration in milliseconds */

/*!
 * A Sensiron command
 */
typedef struct {
  uint16_t cmd;
  uint8_t datalen;
  uint8_t duration;
} sensirion_cmd_t;

/*!
* This structure holds pointer to sensirion sensor specific command set, 
* it's crc configuration and the i2c device structure
*/
typedef struct {
  const sensirion_cmd_t *cmd_set;
  const crc8_cfg_t *crc_config;
  serial_dev_t *dev;
  uint8_t cmd_num;
  const uint8_t cmd_bytes;
} sensirion_device_t;

/*!
* \fn     uint8_t sensirion_set(const sensirion_device_t *device_config, uint8_t cmd, uint16_t *data, int datalen)
* \brief  function acquires the i2c bus, sends given command with given data as parameters added with crc8 for
*         each 2 data bytes and wait for amount of duration (in milliseconds)for command to be processed for 
*         the given sensor. The function releases the i2c bus before returning.
* \return function returns i2c bus status. BUS_STATUS_OK upon success.
*/
uint8_t sensirion_set(const sensirion_device_t *device_config, uint8_t cmd, uint16_t *data, int datalen);

/*!
* \fn     uint8_t sensirion_get(const sensirion_device_t *device_config, uint8_t cmd, uint16_t *data, int datalen)
* \brief  function acquires the i2c bus, sends given command, waits for given duration, read given amount of data,
*         checks crc, remove crc and updates data. The function releases the i2c bus before returning.
* \return function returns i2c bus status. BUS_STATUS_OK upon success.
*/
uint8_t sensirion_get(const sensirion_device_t *device_config, uint8_t cmd, uint16_t *data, int datalen);

/*!
* \fn     uint8_t sensirion_read(const sensirion_device_t *device_config, uint16_t *data, int datalen)
* \brief  function acquires the i2c bus, reads a given amount of data,
*         checks crc, remove crc and updates data. The function releases the i2c bus before returning.
* \return function returns i2c bus status. BUS_STATUS_OK upon success.
*/
uint8_t sensirion_read(const sensirion_device_t *device_config, uint16_t *data, int datalen);
#endif /* SENSIRION_H_ */
