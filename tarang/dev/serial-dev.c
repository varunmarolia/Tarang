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
  if((dev->bus->lock !=0 && dev->bus->current_dev == dev)) {
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
   * so do it outside atmoic section 
   */
  if(!dev->bus->lock && !serial_arch_chip_is_selected(dev)) {
    PRINTF("Selecting chip(%s)\n",__func__);
    serial_arch_chip_select(dev, CHIP_SELECT_ENABLE);
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
    return BUS_BUSY;
  }
  ATOMIC_SECTION(
    /* unlock the bus */
    bus_status = serial_arch_unlock(dev);
  );
  /* disable chip */
  serial_arch_chip_select(dev, CHIP_SELECT_DISBALE);
  return bus_status;
}
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
