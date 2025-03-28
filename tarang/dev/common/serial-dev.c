/**
 * @file serial-dev.c
 * @author Varun Marolia
 * @brief This file contains methods for using Serial devices. The serial device
 *        can be based on UART, SPI or I2C.
 * 
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
#include "serial-dev.h"
#include "serial-status.h"
#include <stddef.h>
#include "atomic.h"

#define DEBUG_SERIAL_DEV 0     /**< Set this to 1 for debug printf output */
#if DEBUG_SERIAL_DEV
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)      /**< Replace printf with nothing */
#endif /* DEBUG_SERIAL_DEV */
/*---------------------------------------------------------------------------*/
bool
serial_dev_has_bus(const serial_dev_t *dev)
{
  if(dev == NULL || dev->bus == NULL) {
    return false;
  }
  if((dev->bus->lock != 0 && dev->bus->current_dev == dev)) {
    return true;
  }
  return false;
}
/*---------------------------------------------------------------------------*/
serial_bus_status_t
serial_dev_bus_acquire(serial_dev_t *dev)
{
  serial_bus_status_t bus_status;
  if(dev == NULL || dev->bus == NULL) {
    return BUS_INVALID;
  }
  if(dev->bus->current_dev == dev && dev->bus->lock) {
    serial_arch_restart_timer(dev);
    return BUS_OK;
  }
  /* Chip select, enable the chip. 
   * The power up time could take few miliseconds,
   * so do it outside atomic section 
   */
  if(dev->cs_config != NULL) {
    if(!dev->bus->lock && !serial_arch_chip_is_selected(dev)) {
      PRINTF("Selecting chip(%s)\n",__func__);
      serial_arch_chip_select(dev, CHIP_SELECT_ENABLE);
    }
  }
  ATOMIC_SECTION(
    bus_status = serial_arch_lock(dev);
  );
  return bus_status;
}
/*---------------------------------------------------------------------------*/
serial_bus_status_t 
serial_dev_bus_release(serial_dev_t *dev)
{
  uint8_t bus_status;
  /* Check to see if given device owns the bus */
  if(!serial_dev_has_bus(dev)) {
    /* The device does not own the bus */
    return BUS_NOT_OWNED;
  }
  ATOMIC_SECTION(
    /* unlock the bus */
    bus_status = serial_arch_unlock(dev);
  );
  /* disable chip */
  if(dev->cs_config != NULL) {
    if(bus_status == BUS_OK) {
      serial_arch_chip_select(dev, CHIP_SELECT_DISBALE);
    }
  }
  return bus_status;
}
/*---------------------------------------------------------------------------*/
serial_bus_status_t
serial_dev_read(serial_dev_t *dev, uint8_t *data, uint16_t size)
{
  serial_bus_status_t bus_status;
  if(dev == NULL || data == NULL || size == 0) {
    return BUS_INVALID;
  }
  if(!serial_dev_has_bus(dev)) {
    return BUS_LOCKED;
  }
  bus_status = serial_arch_read(dev, data, size);
  return bus_status;
}
/*---------------------------------------------------------------------------*/
serial_bus_status_t
serial_dev_write(serial_dev_t *dev, const uint8_t *data, uint16_t size)
{
  serial_bus_status_t bus_status;
  if(dev == NULL || data == NULL || size == 0) {
    return BUS_INVALID;
  }
  if(!serial_dev_has_bus(dev)) {
    return BUS_LOCKED;
  }
  bus_status = serial_arch_write(dev, data, size);
  return bus_status;
}
/*---------------------------------------------------------------------------*/
serial_bus_status_t
serial_dev_transfer(serial_dev_t *dev, const uint8_t *wdata, 
                    uint16_t write_bytes, uint8_t *rdata, uint16_t read_bytes)
{
  serial_bus_status_t bus_status;
  if(dev == NULL || dev->bus == NULL) {
    return BUS_INVALID;
  }
  if(wdata == NULL && write_bytes > 0) {
    return BUS_INVALID;
  }
  if(rdata == NULL && read_bytes > 0) {
    return BUS_INVALID;
  }
  if(!serial_dev_has_bus(dev)) {
    return BUS_LOCKED;
  }
  bus_status = serial_arch_transfer(dev, wdata, write_bytes, rdata, read_bytes);
  return bus_status;
}
/*---------------------------------------------------------------------------*/
void 
serial_dev_chip_select(serial_dev_t *dev, uint8_t on_off)
{
  if(dev == NULL || dev->bus == NULL) {
    return;
  }
  serial_arch_chip_select(dev, on_off);
}
/*---------------------------------------------------------------------------*/
void
serial_dev_set_input_handler(serial_dev_t *dev, void (*handler)(unsigned char c))
{
  if(dev->bus->config.uart_mode == UART_MODE_TX_ONLY) {
    return;
  }
  dev->bus->config.input_handler = handler;
  serial_arch_enable_rx(dev);
}
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
