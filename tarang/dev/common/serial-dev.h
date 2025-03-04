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
typedef struct serial_dev serial_dev_t;

typedef struct serial_bus {
  serial_dev_t *current_dev;    /* pointer to the device currently holding the bus */
  volatile bool lock;           /* lock flag */
  /* arch specific variables */
  serial_bus_config_t config;   /* bus configuration structure containing location, mode, type of bus etc. */
} serial_bus_t;

struct serial_dev {
  serial_bus_t *bus;                /* pointer to current bus */
  uint32_t speed_hz;                /* Bus speed/ baudrate, some devices allow to change clock speed through configuration */
  uint8_t address;                  /* Used only in I2C. Some devices allow to change I2C addresses */
  uint32_t timeout_ms;              /* A timeout could be used to release the lock of the bus and reset */
  uint32_t power_up_delay_ms;       /* time required for this device to power up */
  /* arch specific variables */
  const gpio_config_t *cs_config;   /* chip select configuration */
};

/* Global function calls */
bool serial_dev_has_bus(const serial_dev_t *dev);
serial_bus_status_t serial_dev_bus_acquire(serial_dev_t *dev);
serial_bus_status_t serial_dev_bus_release(serial_dev_t *dev);
serial_bus_status_t serial_dev_read(serial_dev_t *dev, uint8_t *data, uint16_t size);
serial_bus_status_t serial_dev_write(serial_dev_t *dev, const uint8_t *data, uint16_t size);
serial_bus_status_t serial_dev_transfer(serial_dev_t *dev, const uint8_t *wdata, uint16_t write_bytes, uint8_t *rdata, uint16_t read_bytes);
void serial_dev_chip_select(serial_dev_t *dev, uint8_t on_off);
serial_bus_status_t serial_dev_write_byte(serial_dev_t *dev, uint8_t data);
serial_bus_status_t serial_dev_read_byte(serial_dev_t *dev, uint8_t *data);
serial_bus_status_t serial_dev_write_reg(serial_dev_t *dev, uint8_t reg, const uint8_t *data, uint16_t size);
serial_bus_status_t serial_dev_read_reg(serial_dev_t *dev, uint8_t reg, const uint8_t *data, uint16_t size);
void serial_dev_set_input_handler(serial_dev_t *dev, void (*handler)(unsigned char c));

/* Arch specific functions must be implemented in arch specific file */
serial_bus_status_t serial_arch_lock(serial_dev_t *dev);
serial_bus_status_t serial_arch_unlock(serial_dev_t *dev);
void serial_arch_restart_timer(serial_dev_t *dev);
serial_bus_status_t serial_arch_read(serial_dev_t *dev, uint8_t *data, uint16_t len);
serial_bus_status_t serial_arch_write(serial_dev_t *dev, const uint8_t *data, uint16_t len);
serial_bus_status_t serial_arch_transfer(serial_dev_t *dev, const uint8_t *wdata, uint16_t write_bytes, uint8_t *rdata, uint16_t read_bytes);
void serial_arch_chip_select(serial_dev_t *dev, uint8_t on_off);
bool serial_arch_chip_is_selected(serial_dev_t *dev);
void serial_arch_enable_rx(serial_dev_t *dev);
#endif /* _SERIAL_DEV_H_ */
