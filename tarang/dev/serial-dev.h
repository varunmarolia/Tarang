#ifndef _SERIAL_DEV_H_
#define _SERIAL_DEV_H_
#include <stdint.h>
#include "serial-arch.h"

#define I2C_SPEED_NORMAL_HZ   100000
#define I2C_SPEED_FAST_HZ     400000

/* 
 * There could be multiple I2C,SPI,UART buses in a system.
 * Each I2C/SPI bus could have more than one slave devices.
 */
typedef struct serial_bus {
  serial_dev_t *current_dev;  /* pointer the device currently holding the bus */
  volatile bool lock;         /* lock flag */
  serial_bus_config_t config; /* bus configuration structure containing location, mode, type of bus etc. */
} serial_bus_t;

typedef struct serial_dev {
  serial_bus_t *bus;              /* pointer to current bus */
  uint32_t speed_hz;              /* some devices allow to change clock speed through configuration */
  uint8_t address;                /* Used only in I2C. Some devices allow to change I2C addresses */
  uint32_t timeout_ms;            /* A timeout could be used to release the lock of the bus and reset */
  uint32_t power_up_delay_ms;     /* time required for this device to power up */
} serial_dev_t;

/* Golbal function calls */
bool serial_dev_has_bus(const serial_dev_t *dev);
serial_bus_status_t serial_dev_bus_aquire(serial_dev_t *dev);
serial_bus_status_t serial_dev_bus_release(serial_dev_t *dev);
serial_bus_status_t serial_dev_write_byte(serial_dev_t *dev, uint8_t data);
serial_bus_status_t serial_dev_read_byte(serial_dev_t *dev, uint8_t *data);
serial_bus_status_t serial_dev_read(serial_dev_t *dev, const uint8_t *data, uint16_t size);
serial_bus_status_t serial_dev_write(serial_dev_t *dev, const uint8_t *data, uint16_t size);
serial_bus_status_t serial_dev_write_reg(serial_dev_t *dev, uint8_t reg, const uint8_t *data, uint16_t size);
serial_bus_status_t serial_dev_read_reg(serial_dev_t *dev, uint8_t reg, const uint8_t *data, uint16_t size);
#endif /* _SERIAL_DEV_H_ */
