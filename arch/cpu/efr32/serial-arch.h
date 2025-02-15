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

#define SERIAL_BUS_DEFAULT_TIMEOUT_MS   250     /* default bus timeout of 500 mseconds */
#define SERIAL_UART_DEFAUT_BAUDRATE     115200
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

typedef enum {
UART_MODE_TX_ONLY = 0,
UART_MODE_TX_RX = 1,
UART_MODE_TX_RX_FLOW = 2
} uart_mode_t;
typedef struct serial_dev_cs_config {
  const GPIO_Port_TypeDef port;
  const uint8_t pin;
  const enable_logic_t logic;
  const bool auto_cs;
} serial_dev_cs_config_t;

typedef struct serial_bus_config {
  const uint32_t data_in_loc;         /* SPI:MISO, I2C:SDA, UART:RX */
  const uint32_t data_out_loc;        /* SPI:MOSI, I2C:SDA, UART:TX */
  const uint32_t clk_loc;             /* SPI:Clk, I2C:Clk, UART: don't care */
  const uint32_t cts_loc;             /* uart signal clear to send */
  const uint32_t rts_loc;             /* uart signal ready to send */
  const uart_mode_t uart_mode;        /* UART mode */
  uint8_t (* uart_input_handler)(unsigned char c); /* input handler for UART */
  const bus_type_t type;              /* type of bus i.e. I2C, SPI or UART */
  I2C_TypeDef *I2Cx;                  /* pointer to I2C bus address */  
  USART_TypeDef *SPI_UART_USARTx;     /* pointer to SPI/USART/UART bus address */
  USART_ClockMode_TypeDef clock_mode; /* SPI clock mode, e.g. idle low, sample on rising edge */
  bool msb_first;                     /* MSB goes out first */
  timer_t bus_timer;                  /* timer for the bus used in bus timeout */
} serial_bus_config_t;

#endif /* _SERIAL_BUS_ARCH_H_ */