/*!
*\file  sensirion.c
*\brief This file holds low level function bodies those are common 
*       between different sensirion sensors (e.g sht3x, sgpc3, sps30) in I2C mode.
*/
#include "timer.h"
#include "sensirion.h"

#define DEBUG_SENSIRION  0 /**< This macro enables/disables the debug printf for this file */

#if DEBUG_SENSIRION
#include <stdio.h>
#undef PRINTF
#define PRINTF(...) printf(__VA_ARGS__)
#else  /* DEBUG_SENSIRION */
/**< Replace printf with nothing */
#define PRINTF(...)
#endif /* DEBUG_SENSIRION */

/*---------------------------------------------------------------------------*/
uint8_t
sensirion_set(const sensirion_device_t *device_config, uint8_t cmd, uint16_t *data, int datalen)
{
  serial_bus_status_t bus_status;
  uint8_t i;
  uint8_t sensirion_cmd[SENSIRION_MAX_SET_PARAM_LENGTH + 2];
  uint8_t *sensirion_data;

  /* Sanity check data and datalen first */
  if(cmd > device_config->cmd_num
     || (device_config->cmd_set[cmd].datalen == 0 && (data != NULL || datalen != 0))
     || (device_config->cmd_set[cmd].datalen > 0
         && (data == NULL || datalen * 3 != device_config->cmd_set[cmd].datalen))
     || !device_config->cmd_bytes || device_config->cmd_bytes > 2) {
    PRINTF("SENSIRION invalid cmd/data/datalen\n");
    return BUS_INVALID;
  }
  if(SENSIRION_MAX_SET_PARAM_LENGTH < device_config->cmd_set[cmd].datalen) {
    PRINTF("SENSIRION invalid param length\n");
    return BUS_INVALID;
  }
  
  /* first 1 or 2 bytes are commands the rest are data/parameters */
  sensirion_data = sensirion_cmd + device_config->cmd_bytes;
  
  bus_status = serial_dev_bus_acquire(device_config->dev);
  if(bus_status != BUS_OK) {
    PRINTF("SENSIRION couldn't acquire bus %u\n", bus_status);
    return bus_status;
  }
  if(device_config->cmd_bytes == 1) {
    sensirion_cmd[0] = (uint8_t) device_config->cmd_set[cmd].cmd;
  } else {
    sensirion_cmd[0] = (uint8_t) (device_config->cmd_set[cmd].cmd >> 8);
    sensirion_cmd[1] = (uint8_t) device_config->cmd_set[cmd].cmd;
  }
  /* Make sure the data pointer is not NULL */
  if(data) {
    for(i = 0; i < device_config->cmd_set[cmd].datalen; i += 3) {
      sensirion_data[i] = (uint8_t) (data[i / 3] >> 8);        /* MSB first */
      sensirion_data[i + 1] = (uint8_t) data[i / 3];           /* LSB */
      sensirion_data[i + 2] = crc8_calc_buff(device_config->crc_config, sensirion_data + i, 2);  /* calculate CRC for last 2 bytes */
    }
  }

  /* write the command buffer on i2c bus */
  bus_status = serial_dev_write(device_config->dev, sensirion_cmd, device_config->cmd_set[cmd].datalen + 2);
  if(bus_status != BUS_OK) {
    PRINTF("SENSIRION failed to write set command %u\n", bus_status);
    serial_dev_bus_release(device_config->dev);
    return bus_status;
  }

  /* wait for the response time for this command */
  clock_wait_ms(device_config->cmd_set[cmd].duration);

  return serial_dev_bus_release(device_config->dev);
}
/*---------------------------------------------------------------------------*/
uint8_t
sensirion_read(const sensirion_device_t *device_config, uint16_t *data, int datalen)
{
  serial_bus_status_t bus_status;
  uint8_t i;
  uint8_t crc;
  uint8_t sensirion_data[SENSIRION_MAX_GET_PARAM_LENGTH];

  if(SENSIRION_MAX_GET_PARAM_LENGTH < datalen * 3) {
    PRINTF("SENSIRION invalid datalen\n");
    return BUS_INVALID;
  }

  /* acquire I2C bus and proceed with I2C commands */
  bus_status = serial_dev_bus_acquire(device_config->dev);
  if(bus_status != BUS_OK) {
    PRINTF("SENSIRION couldn't acquire bus %u\n", bus_status);
    return bus_status;
  }

  /* read response */
  bus_status = serial_dev_read(device_config->dev, sensirion_data, datalen * 3);
  if(bus_status != BUS_OK) {
    PRINTF("SENSIRION failed to read get response %u\n", bus_status);
    serial_dev_bus_release(device_config->dev);
    return bus_status;
  }

  /* check and remove CRC. move read data to 16 bit data pointer */
  if(data != NULL) {
    for(i = 0; i < datalen * 3; i += 3) {
      /* calculate CRC for next 2 bytes */
      crc = crc8_calc_buff(device_config->crc_config, sensirion_data + i, 2);
      /* compare calculated CRC with received CRC */
      if(sensirion_data[i + 2] == crc) {
        data[i / 3] = sensirion_data[i] << 8;  /* MSB first */
        data[i / 3] |= sensirion_data[i + 1];  /* LSB */
      } else {
        PRINTF("SENSIRION CRC failed!!!\n");
        serial_dev_bus_release(device_config->dev);
        return BUS_DATA_NACK;
      }
    }
  }

  return serial_dev_bus_release(device_config->dev);
}
/*---------------------------------------------------------------------------*/
uint8_t
sensirion_get(const sensirion_device_t *device_config, uint8_t cmd, uint16_t *data, int datalen)
{
  serial_bus_status_t bus_status;
  uint8_t sensirion_cmd[2];

  /* Sanity check data and datalen first */
  if((device_config->cmd_set[cmd].datalen == 0 && (data != NULL || datalen != 0))
     || (device_config->cmd_set[cmd].datalen > 0
         && (data == NULL || datalen * 3 != device_config->cmd_set[cmd].datalen))
     || !device_config->cmd_bytes || device_config->cmd_bytes > 2) {
    PRINTF("SENSIRION invalid data/datalen\n");
    return BUS_INVALID;
  }

  if(SENSIRION_MAX_GET_PARAM_LENGTH < device_config->cmd_set[cmd].datalen) {
    PRINTF("SENSIRION invalid param length\n");
    return BUS_INVALID;
  }

  /* acquire I2C bus and proceed with I2C commands */
  bus_status = serial_dev_bus_acquire(device_config->dev);
  if(bus_status != BUS_OK) {
    PRINTF("SENSIRION couldn't acquire bus %u\n", bus_status);
    return bus_status;
  }
  if(device_config->cmd_bytes == 1) {
    sensirion_cmd[0] = (uint8_t) device_config->cmd_set[cmd].cmd;
  } else {
    sensirion_cmd[0] = (uint8_t) (device_config->cmd_set[cmd].cmd >> 8);
    sensirion_cmd[1] = (uint8_t) device_config->cmd_set[cmd].cmd;
  }
  /* write the command to get data */
  bus_status = serial_dev_write(device_config->dev, sensirion_cmd, device_config->cmd_bytes);
  if(bus_status != BUS_OK) {
    PRINTF("SENSIRION failed to write get command %u\n", bus_status);
    serial_dev_bus_release(device_config->dev);
    return bus_status;
  }

  /* wait for the response time for this command */
  clock_wait_ms(device_config->cmd_set[cmd].duration);
  /* release the I2C bus */
  serial_dev_bus_release(device_config->dev);
  /* read response */
  return sensirion_read(device_config, data, datalen);
}
/*---------------------------------------------------------------------------*/