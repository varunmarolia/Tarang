#ifndef _SERIAL_BUS_ARCH_H_
#define _SERIAL_BUS_ARCH_H_
#include <stdint.h>
#include <stddef.h>
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_i2c.h"
#include "em_usart.h"
#include "serial-status.h"
#include "timer.h"
#include "clock.h"

#define SERIAL_BUS_DEFAULT_TIMEOUT_MS   250   /* default bus timeout of 500 mseconds */
#define CHIP_SELECT_ENABLE 0
#define CHIP_SELECT_DISBALE 1

typedef enum bus_type {
BUS_TYPE_I2C = 0,
BUS_TYPE_SPI = 1,
BUS_TYPE_UART = 2
} bus_type_t;

typedef enum enable_logic {
ENABLE_ACTIVE_LOW = 0,
ENABLE_ACTIVE_HIGH = 1
} enable_logic_t;

typedef struct serial_bus_config {
  const uint32_t data_in_loc;
  const uint32_t data_out_loc;
  const uint32_t clk_loc;
  const GPIO_Port_TypeDef chip_select_port;
  const uint8_t chip_select_pin;
  const enable_logic_t chip_select_logic;
  const bus_type_t type;
  I2C_TypeDef *I2Cx;
  USART_TypeDef *SPI_USARTx;
  USART_ClockMode_TypeDef clock_mode;
  bool msb_first;
  timer_t bus_timer;
} serial_bus_config_t;

#endif /* _SERIAL_BUS_ARCH_H_ */