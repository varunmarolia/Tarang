#include "serial-arch.h"
#include "serial-dev.h"
#include <stdbool.h>

#define RX_NVIC 0
#define TX_NVIC 1

#define DEBUG_SERIAL_ARCH 0     /**< Set this to 1 for debug printf output */
#if DEBUG_SERIAL_ARCH
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)      /**< Replace printf with nothing */
#endif /* DEBUG_SERIAL_ARCH */

/*---------------------------------------------------------------------------*/
static serial_bus_status_t
serial_init_I2C(serial_dev_t *dev)
{
  serial_bus_config_t *bus_config = &dev->bus->config;
  uint32_t scl_port = 0, scl_pin = 0;
  uint32_t sda_port = 0, sda_pin = 0;
  I2C_Init_TypeDef i2cInit = I2C_INIT_DEFAULT;
  /* check if the speed is configured correctly. Usual speed are 100KHz or 400KHz */
  if(dev->speed_hz != I2C_SPEED_NORMAL_HZ &&
    dev->speed_hz != I2C_SPEED_FAST_HZ) {
    PRINTF("I2C (%s): speed %lu is invalid\n", __func__, dev->speed);
    return BUS_INVALID;
  }
  /* Check if the I2C bus is supported by the hardware
  * enable required clocks (GPIO for pins routing) for given I2C bus
  * Decode port and pin number using pin locations.
  * */
  if(bus_config->I2Cx == I2C0) {
    CMU_ClockEnable(cmuClock_I2C0, true);
    /* unlock I2C peripherals and grant access
    * to their registers for updating their configurations after a deep sleep.
    */
    EMU->EM23PERNORETAINCMD = EMU_EM23PERNORETAINCMD_I2C0UNLOCK;
    /* for I2C data out and data in location must be same */
    sda_port = AF_I2C0_SDA_PORT(bus_config->data_out_loc);
    sda_pin = AF_I2C0_SDA_PIN(bus_config->data_out_loc);

    scl_port = AF_I2C0_SCL_PORT(bus_config->clk_loc);
    scl_pin = AF_I2C0_SCL_PIN(bus_config->clk_loc);

  #ifdef I2C1
  } else if(bus_config->I2Cx == I2C1) {
    CMU_ClockEnable(cmuClock_I2C1, true);
    /* unlock I2C peripherals and grant access
    * to their registers for updating their configurations after a deep sleep.
    */
    EMU->EM23PERNORETAINCMD = EMU_EM23PERNORETAINCMD_I2C1UNLOCK;

    sda_port = AF_I2C1_SDA_PORT(bus_config->data_out_loc);
    sda_pin = AF_I2C1_SDA_PIN(bus_config->data_out_loc);

    scl_port = AF_I2C1_SCL_PORT(bus_config->clk_loc);
    scl_pin = AF_I2C1_SCL_PIN(bus_config->clk_loc);
  #endif /* I2C1 */
  } else {
    PRINTF("I2C (%s): unsupported I2Cx: %p\n", __func__, conf->I2Cx);
    return BUS_INVALID;
  }
  PRINTF("I2C: SDA PORT: %d, PIN: %d\n", (int)sda_port, (int)sda_pin);
  PRINTF("I2C: SCL PORT: %d, PIN: %d\n", (int)scl_port, (int)scl_pin);
  /* enable GPIO clock  */
  CMU_ClockEnable(cmuClock_GPIO, true);
  /* Set the GPIO pin mode Open-drain output with filter */
  GPIO_PinModeSet(sda_port, sda_pin, gpioModeWiredAndFilter, 1);
  GPIO_PinModeSet(scl_port, scl_pin, gpioModeWiredAndFilter, 1);
  /* Route the given I2C bus pins to given pin location  */
  bus_config->I2Cx->ROUTEPEN = I2C_ROUTEPEN_SDAPEN | I2C_ROUTEPEN_SCLPEN;
  bus_config->I2Cx->ROUTELOC0 = (bus_config->data_out_loc << _I2C_ROUTELOC0_SDALOC_SHIFT) |
                          (bus_config->clk_loc << _I2C_ROUTELOC0_SCLLOC_SHIFT);
  i2cInit.enable = true; /* enable the bus */
  i2cInit.master = true; /* master mode only */
  /* initialize I2C controller */
  I2C_Init(bus_config->I2Cx, &i2cInit);
  /* Set bus frequency */
  I2C_BusFreqSet(bus_config->I2Cx, 0, dev->speed_hz, i2cClockHLRStandard);
  /* setup auto acknowledge and auto STOP on NACK */
  bus_config->I2Cx->CTRL |= I2C_CTRL_AUTOACK | I2C_CTRL_AUTOSN;
  /* setup the i2c bus timeout for this device */
  serial_arch_restart_timer(dev);
  /* lock the device by setting the flag and pairing the device pointer */
  dev->bus->lock = 1;
  dev->bus->current_dev = dev;
  return BUS_OK;
}
/*---------------------------------------------------------------------------*/
static serial_bus_status_t
serial_init_SPI(serial_dev_t *dev)
{
  USART_TypeDef* spi_uart = dev->bus->config.SPI_UART_USARTx;
  serial_bus_config_t* bus_config = &(dev->bus->config);

  /* enable all required clocks */
  if(spi_uart == USART0) {
    CMU_ClockEnable(cmuClock_USART0, true);
  } else if(spi_uart == USART1) {
    CMU_ClockEnable(cmuClock_USART1, true);
  } else if(spi_uart == USART2) {
    CMU_ClockEnable(cmuClock_USART2, true);
  } else {
    PRINTF("SPI (%s): interface not supported!\n", __func__);
    return BUS_INVALID;
  }
  CMU_ClockEnable(cmuClock_GPIO, true);

  USART_InitSync_TypeDef spi_init = USART_INITSYNC_DEFAULT;

  /* configure SPI controller */
  spi_init.master = true;
  spi_init.msbf     = bus_config->msb_first; 
  spi_init.baudrate = dev->speed_hz;
  spi_init.clockMode = bus_config->clock_mode;
  USART_InitSync(spi_uart, &spi_init);

  /* IO config */
  if(spi_uart == USART0) {
    GPIO_PinModeSet(AF_USART0_TX_PORT(bus_config->data_out_loc), AF_USART0_TX_PIN(bus_config->data_out_loc), gpioModePushPull, 1);
    GPIO_PinModeSet(AF_USART0_RX_PORT(bus_config->data_in_loc), AF_USART0_RX_PIN(bus_config->data_in_loc), gpioModeInput, 0);
    GPIO_PinModeSet(AF_USART0_CLK_PORT(bus_config->clk_loc), AF_USART0_CLK_PIN(bus_config->clk_loc), gpioModePushPull, 1);
  } else if(spi_uart == USART1) {
    GPIO_PinModeSet(AF_USART1_TX_PORT(bus_config->data_out_loc), AF_USART1_TX_PIN(bus_config->data_out_loc), gpioModePushPull, 1);
    GPIO_PinModeSet(AF_USART1_RX_PORT(bus_config->data_in_loc), AF_USART1_RX_PIN(bus_config->data_in_loc), gpioModeInput, 0);
    GPIO_PinModeSet(AF_USART1_CLK_PORT(bus_config->clk_loc), AF_USART1_CLK_PIN(bus_config->clk_loc), gpioModePushPull, 1);
  } else if(spi_uart == USART2) {
    GPIO_PinModeSet(AF_USART2_TX_PORT(bus_config->data_out_loc), AF_USART2_TX_PIN(bus_config->data_out_loc), gpioModePushPull, 1);
    GPIO_PinModeSet(AF_USART2_RX_PORT(bus_config->data_in_loc), AF_USART2_RX_PIN(bus_config->data_in_loc), gpioModeInput, 0);
    GPIO_PinModeSet(AF_USART2_CLK_PORT(bus_config->clk_loc), AF_USART2_CLK_PIN(bus_config->clk_loc), gpioModePushPull, 1);
  } else {
    PRINTF("SPI (%s): controller not supported!\n", __func__);
    return BUS_INVALID;
  }

  /* enable routing in USART controller */
  spi_uart->ROUTELOC0 = ((bus_config->data_out_loc << _USART_ROUTELOC0_TXLOC_SHIFT) |
                       (bus_config->data_in_loc << _USART_ROUTELOC0_RXLOC_SHIFT) |
                       (bus_config->clk_loc << _USART_ROUTELOC0_CLKLOC_SHIFT));
  spi_uart->ROUTEPEN = (USART_ROUTEPEN_RXPEN | USART_ROUTEPEN_TXPEN | USART_ROUTEPEN_CLKPEN);

  /* setup the bus timeout for this device */
  serial_arch_restart_timer(dev);
  /* lock the device by setting the flag and pairing the device pointer */
  dev->bus->lock = 1;
  dev->bus->current_dev = dev;
  return BUS_OK;
}
/*---------------------------------------------------------------------------*/
static void
enable_NVIC_interrupt(USART_TypeDef *uart, uint8_t rxtx)
{
  if(rxtx == RX_NVIC) {
    if(uart == USART0) {
      NVIC_EnableIRQ(USART0_RX_IRQn);
    } else if(uart == USART1) {
      NVIC_EnableIRQ(USART1_RX_IRQn);
    } else if(uart == USART2) {
      NVIC_EnableIRQ(USART2_RX_IRQn);
    }
  } else {
    if(uart == USART0) {
      NVIC_EnableIRQ(USART0_TX_IRQn);
    } else if(uart == USART1) {
      NVIC_EnableIRQ(USART1_TX_IRQn);
    } else if(uart == USART2) {
      NVIC_EnableIRQ(USART2_TX_IRQn);
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
clear_NVIC_pending(USART_TypeDef *uart, uint8_t rxtx)
{
  if(rxtx == RX_NVIC) {
    if(uart == USART0) {
      NVIC_ClearPendingIRQ(USART0_RX_IRQn);
    } else if(uart == USART1) {
      NVIC_ClearPendingIRQ(USART1_RX_IRQn);
    } else if(uart == USART2) {
      NVIC_ClearPendingIRQ(USART2_RX_IRQn);
    }
  } else {
    if(uart == USART0) {
      NVIC_ClearPendingIRQ(USART0_TX_IRQn);
    } else if(uart == USART1) {
      NVIC_ClearPendingIRQ(USART1_TX_IRQn);
    } else if(uart == USART2) {
      NVIC_ClearPendingIRQ(USART2_TX_IRQn);
    }
  }
}
/*---------------------------------------------------------------------------*/
static serial_bus_status_t
serial_init_UART(serial_dev_t *dev)
{
  serial_bus_config_t *bus_config = &dev->bus->config;
  USART_InitAsync_TypeDef uart_init  = USART_INITASYNC_DEFAULT;
  CMU_ClockEnable(cmuClock_HFPER, true);
  CMU_ClockEnable(cmuClock_GPIO, true);
  GPIO_Port_TypeDef tx_port, rts_port;
  GPIO_Port_TypeDef rx_port, cts_port;
  uint8_t tx_pin, rts_pin;
  uint8_t rx_pin, cts_pin;
  /* if special baud rate is defined load that. default is 115200 */
  if(dev->speed_hz != 0) {
    uart_init.baudrate = dev->speed_hz;
  } else {
    uart_init.baudrate = SERIAL_UART_DEFAUT_BAUDRATE;
  }

  if(bus_config->SPI_UART_USARTx == USART0) {
    CMU_ClockEnable(cmuClock_USART0, true);
    tx_port = AF_USART0_TX_PORT(bus_config->data_out_loc);
    tx_pin = AF_USART0_TX_PIN(bus_config->data_out_loc);
    rx_port = AF_USART0_RX_PORT(bus_config->data_in_loc);
    rx_pin = AF_USART0_RX_PIN(bus_config->data_in_loc);
    if(bus_config->uart_mode == UART_MODE_TX_RX_FLOW) {
      rts_port = AF_USART0_RTS_PORT(bus_config->rts_loc);
      rts_pin = AF_USART0_RTS_PIN(bus_config->rts_loc);
      cts_port = AF_USART0_CTS_PORT(bus_config->cts_loc);
      cts_pin = AF_USART0_CTS_PIN(bus_config->cts_loc);
    }
  }
  else if(bus_config->SPI_UART_USARTx == USART1) {
    CMU_ClockEnable(cmuClock_USART1, true);
    tx_port = AF_USART1_TX_PORT(bus_config->data_out_loc);
    tx_pin = AF_USART1_TX_PIN(bus_config->data_out_loc);
    rx_port = AF_USART1_RX_PORT(bus_config->data_in_loc);
    rx_pin = AF_USART1_RX_PIN(bus_config->data_in_loc);
    if(bus_config->uart_mode == UART_MODE_TX_RX_FLOW) {
      rts_port = AF_USART1_RTS_PORT(bus_config->rts_loc);
      rts_pin = AF_USART1_RTS_PIN(bus_config->rts_loc);
      cts_port = AF_USART1_CTS_PORT(bus_config->cts_loc);
      cts_pin = AF_USART1_CTS_PIN(bus_config->cts_loc);
    }
  }
  else if(bus_config->SPI_UART_USARTx == USART2) {
    CMU_ClockEnable(cmuClock_USART2, true);
    tx_port = AF_USART2_TX_PORT(bus_config->data_out_loc);
    tx_pin = AF_USART2_TX_PIN(bus_config->data_out_loc);
    rx_port = AF_USART2_RX_PORT(bus_config->data_in_loc);
    rx_pin = AF_USART2_RX_PIN(bus_config->data_in_loc);
    if(bus_config->uart_mode == UART_MODE_TX_RX_FLOW) {
      rts_port = AF_USART2_RTS_PORT(bus_config->rts_loc);
      rts_pin = AF_USART2_RTS_PIN(bus_config->rts_loc);
      cts_port = AF_USART2_CTS_PORT(bus_config->cts_loc);
      cts_pin = AF_USART2_CTS_PIN(bus_config->cts_loc);
    }
  }
  else {
    PRINTF("UART (%s): interface not supported!\n", __func__);
    return BUS_INVALID;
  }
  /* Configure GPIO pin, To avoid false start, configure TX pin as initial high */
  GPIO_PinModeSet(tx_port, tx_pin, gpioModePushPull, 1);
  if(bus_config->uart_mode != UART_MODE_TX_ONLY) {
    GPIO_PinModeSet(rx_port, rx_pin, gpioModeInput, 0);
  }
  /* set RTS and CTS pin mode if they are defined */
  if(bus_config->uart_mode == UART_MODE_TX_RX_FLOW) {
    uart_init.hwFlowControl = usartHwFlowControlCtsAndRts;
    GPIO_PinModeSet(rts_port, rts_pin, gpioModePushPull, 1);
    GPIO_PinModeSet(cts_port, cts_pin, gpioModeInput, 0);
  }

  /* Don't enable uart upon initialization */
  uart_init.enable = usartDisable;
  /* Initialize the UART in async mode */
  USART_InitAsync(bus_config->SPI_UART_USARTx, &uart_init);
  /* Raise TXBL interrupt as soon as there is at least one empty slot */
  bus_config->SPI_UART_USARTx->CTRL |= USART_CTRL_TXBIL;

  if(bus_config->uart_mode != UART_MODE_TX_ONLY) {
    bus_config->SPI_UART_USARTx->ROUTELOC0 = (bus_config->SPI_UART_USARTx->ROUTELOC0 
                                            & ~(_USART_ROUTELOC0_TXLOC_MASK | _USART_ROUTELOC0_RXLOC_MASK ))
                                            | (bus_config->data_out_loc << _USART_ROUTELOC0_TXLOC_SHIFT)
                                            | (bus_config->data_in_loc << _USART_ROUTELOC0_RXLOC_SHIFT);
    bus_config->SPI_UART_USARTx->ROUTEPEN |= USART_ROUTEPEN_RXPEN | USART_ROUTEPEN_TXPEN;
  } else {
    bus_config->SPI_UART_USARTx->ROUTELOC0 = (bus_config->SPI_UART_USARTx->ROUTELOC0 
                                            & ~(_USART_ROUTELOC0_TXLOC_MASK | _USART_ROUTELOC0_RXLOC_MASK ))
                                            | (bus_config->data_out_loc << _USART_ROUTELOC0_TXLOC_SHIFT);
    bus_config->SPI_UART_USARTx->ROUTEPEN |= USART_ROUTEPEN_TXPEN;
  }

  if(bus_config->uart_mode == UART_MODE_TX_RX_FLOW) {
    bus_config->SPI_UART_USARTx->ROUTELOC1 = ((bus_config->rts_loc << _USART_ROUTELOC1_RTSLOC_SHIFT) 
                                              | (bus_config->cts_loc << _USART_ROUTELOC1_CTSLOC_SHIFT));
    bus_config->SPI_UART_USARTx->ROUTEPEN |= USART_ROUTEPEN_RTSPEN | USART_ROUTEPEN_CTSPEN;
  }
  /* clear any uart peripheral interrupt */
  USART_IntClear(bus_config->SPI_UART_USARTx, _USART_IF_MASK);
  /* Clear any pending IRQ request for UART */
  clear_NVIC_pending(bus_config->SPI_UART_USARTx, RX_NVIC);
  clear_NVIC_pending(bus_config->SPI_UART_USARTx, TX_NVIC);
  USART_Enable(bus_config->SPI_UART_USARTx, usartEnable);        /* Enable transmit and receive */
  USART_IntEnable(bus_config->SPI_UART_USARTx, USART_IF_TXBL);   /* Enable transmit buffer empty interrupt */

  /* DEBUG_USART_TX_IRQn will be enabled in NVIC when there is data to send */
  return BUS_OK;
}
/*---------------------------------------------------------------------------*/
static serial_bus_status_t 
i2c_return_to_bus_status(I2C_TransferReturn_TypeDef i2c_ret)
{
  switch (i2c_ret)
  {
  case i2cTransferInProgress:
    return BUS_TIMEOUT;
    break;
  case i2cTransferDone:
    return BUS_OK;
    break;
  case i2cTransferNack:
    return BUS_DATA_NACK;
    break;
  case i2cTransferBusErr:
  case i2cTransferArbLost:
  case i2cTransferUsageFault:
  case i2cTransferSwFault:
    return BUS_UNKNOWN_ERROR;
    break;
  default:
    return BUS_UNKNOWN_ERROR;
    break;
  }
}
/*---------------------------------------------------------------------------*/
void 
serial_arch_chip_select(serial_dev_t *dev, uint8_t on_off)
{
  if(!dev->cs_config.auto_cs) {
    return;
  }
  GPIO_PinModeSet(dev->cs_config.port, dev->cs_config.pin, gpioModePushPull, 1);
  if (on_off == CHIP_SELECT_ENABLE) {
    if(dev->cs_config.logic == ENABLE_ACTIVE_LOW) {
      GPIO_PinOutClear(dev->cs_config.port , dev->cs_config.pin);
    } else {
      GPIO_PinOutSet(dev->cs_config.port , dev->cs_config.pin);
    }
    if(dev->power_up_delay_ms) {
      clock_wait_ms(dev->power_up_delay_ms);
    }
  } else {
    if(dev->cs_config.logic == ENABLE_ACTIVE_LOW) {
      GPIO_PinOutSet(dev->cs_config.port , dev->cs_config.pin);
    } else {
      GPIO_PinOutClear(dev->cs_config.port , dev->cs_config.pin);
    }
  }
}
/*---------------------------------------------------------------------------*/
bool
serial_arch_chip_is_selected(serial_dev_t *dev)
{
  bool cs_enabled;
  
  if(!dev->cs_config.auto_cs) {
    return false;
  }

  cs_enabled = GPIO_PinOutGet(dev->cs_config.port, dev->cs_config.pin);
  if(dev->cs_config.logic == ENABLE_ACTIVE_LOW) {
    cs_enabled = !cs_enabled;
  }

  return cs_enabled;
}
/*---------------------------------------------------------------------------*/
serial_bus_status_t 
serial_arch_lock(serial_dev_t *dev)
{
  serial_bus_config_t *bus_config = &dev->bus->config;
  serial_bus_status_t bus_status = BUS_OK;
  /* check if bus is already locked by some other device */
  if(dev->bus->lock && dev->bus->current_dev != dev) {
    /* if timer timedout for the device holding the bus, unlock the bus */
    if(dev->timeout_ms && timer_timedout(&bus_config->bus_timer)) {
      serial_arch_unlock(dev);
    } else {
      PRINTF("Serial bus (%s): bus is locked\n", __func__);
      return BUS_LOCKED;
    }
  }
  /* if bus is not locked, initialize the device */
  if(!dev->bus->lock) {
    switch(bus_config->type) {
      case BUS_TYPE_I2C:
        /* Initialize the bus */
        bus_status = serial_init_I2C(dev);
      break;
      case BUS_TYPE_SPI:
        bus_status = serial_init_SPI(dev);
      break;
      case BUS_TYPE_UART:
        bus_status = serial_init_UART(dev);
      break;
      default:
        PRINTF("Serial bus (%s): wrong bus type\n", __func__);
      break;
    }
  }
  return bus_status;
}
/*---------------------------------------------------------------------------*/
serial_bus_status_t
serial_arch_unlock(serial_dev_t *dev)
{
  serial_bus_config_t *bus_config = &dev->bus->config;
  if(dev->bus->lock) {
    if(dev->bus->current_dev == dev) {
      switch(bus_config->type) {
        case BUS_TYPE_I2C:
        /* Reset the I2C controller */
        I2C_Reset(dev->bus->config.I2Cx);
        /* turn off clocks to reduce power consumption */
        if(bus_config->I2Cx == I2C0) {
          CMU_ClockEnable(cmuClock_I2C0, false);
        }
#ifdef I2C1
        if(bus_config->I2Cx == I2C1) {
          CMU_ClockEnable(cmuClock_I2C1, false);
        }
#endif  /* I2C1 */
        break;
        case BUS_TYPE_SPI:
          /* Reset the SPI controller */
          USART_Reset(dev->bus->config.SPI_UART_USARTx);
          /* turn off clocks to reduce power consumption */
          if(bus_config->SPI_UART_USARTx == USART0) {
            CMU_ClockEnable(cmuClock_USART0, false);
          }
          if(bus_config->SPI_UART_USARTx == USART1) {
            CMU_ClockEnable(cmuClock_USART1, false);
          }
          if(bus_config->SPI_UART_USARTx == USART2) {
            CMU_ClockEnable(cmuClock_USART2, false);
          }
        break;
        case BUS_TYPE_UART:
        break;
        default:
          PRINTF("Serial bus (%s): wrrong bus typr\n", __func__);
        break;
      } 
    } else {
      return BUS_NOT_OWNED;
    }
    /* unlock the bus */
    dev->bus->lock = 0;
    dev->bus->current_dev = NULL;
  }
  return BUS_OK;
}
/*---------------------------------------------------------------------------*/
void
serial_arch_restart_timer(serial_dev_t *dev)
{
  serial_bus_config_t *bus_config = &dev->bus->config;
  if(dev->timeout_ms) {
    timer_set(&bus_config->bus_timer, dev->timeout_ms);
  } else {
    timer_set(&bus_config->bus_timer, SERIAL_BUS_DEFAULT_TIMEOUT_MS);
  }
}
/*---------------------------------------------------------------------------*/
serial_bus_status_t 
serial_arch_transfer(serial_dev_t *dev, const uint8_t *wdata, 
                    uint16_t write_bytes, uint8_t *rdata, uint16_t read_bytes)
{
  serial_bus_config_t *bus_config = &dev->bus->config;
  uint16_t max_data_lenght;
  uint16_t i = 0;
  uint8_t byte;
  I2C_TransferReturn_TypeDef i2c_ret;
  I2C_TransferSeq_TypeDef i2cTransfer;

  if(dev->bus->lock) {
    if(dev->bus->current_dev == dev) {
      switch(bus_config->type) {
        case BUS_TYPE_I2C:
          /* Initialize the i2c transfer structure, use buf[0] to transfer data  */
        i2cTransfer.addr = dev->address;   /* 7-bit address */
        if(rdata && read_bytes) {
          i2cTransfer.flags = I2C_FLAG_READ;
          i2cTransfer.buf[0].data = rdata;       /* pointer to data where read data will be kept */
          i2cTransfer.buf[0].len = read_bytes;   /* Number of bytes to read */
        } else {
          i2cTransfer.flags = I2C_FLAG_WRITE;
          i2cTransfer.buf[0].data = (uint8_t *)wdata; /* pointer to data */
          i2cTransfer.buf[0].len = write_bytes;       /* Number of bytes to write */
        }
        if(bus_config->I2Cx == NULL) {
          return BUS_INVALID;
        }
        /* prepare and start I2C transfer. Call to I2C_TransferInit must 
        *  be made for starting the transfer in master mode */
        i2c_ret = I2C_TransferInit(bus_config->I2Cx, &i2cTransfer);
        if(i2c_ret != i2cTransferDone && i2c_ret != i2cTransferInProgress) {
          return i2c_return_to_bus_status(i2c_ret);
        }
        /* poll using I2C_Transfer function  */
        while(i2c_ret == i2cTransferInProgress && !timer_timedout(&bus_config->bus_timer)) {
#if defined(efr32bg13p) || defined(EFR32BG13P732F512GM48) || defined(efr32xg13)
          if(rdata && read_bytes) {
            /* from efr32xg13-errata R1.1 pdf
            * Errata I2C_E207- I2C Fails to Indicate New Incoming Data.
            *
            * Impacts - A race condition exists in which the I2C fails to indicate reception of 
            * new data when both user software attempts to read data from and
            * the I2C hardware attempts to write data to the I2C_RXFIFO in the same cycle.
            * 
            * Workaround - User software can recognize and clear this invalid RXDATAV = 0 
            * and RXFULL = 1 condition by performing a dummy read of the RXFIFO (I2C_RXDATA).
            */
            if(!(bus_config->I2Cx->STATUS & I2C_STATUS_RXDATAV) &&
              (bus_config->I2Cx->STATUS & I2C_STATUS_RXFULL)) {
                bus_config->I2Cx->RXDATA; /* perform dummy read to fix the issue */
              }
          }
#endif  /* defined(efr32bg13p) || defined(EFR32BG13P732F512GM48) */
          i2c_ret = I2C_Transfer(dev->bus->config.I2Cx);  /* Call to I2C_Transfer must be made to complete the transfer in master mode */
        }
        if(i2c_ret != i2cTransferDone) {
          return i2c_return_to_bus_status(i2c_ret);
        }
        break;
        
        case BUS_TYPE_SPI:
          /* sanity check for read/write data  */
          if(wdata == NULL && write_bytes == 0) {
            return BUS_INVALID;
          }
          if(rdata == NULL && read_bytes == 0) {
            return BUS_INVALID;
          }
          max_data_lenght = (read_bytes > write_bytes) ? read_bytes : write_bytes;
          if(!max_data_lenght) {
            return BUS_OK;
          }
          /* transfer the data. could be read, write or read + write */
          for(i = 0; i < max_data_lenght; i++) {
            byte = (i < write_bytes) ? wdata[i] : 0;
            byte = USART_SpiTransfer(bus_config->SPI_UART_USARTx, byte);
            if(i < read_bytes) {
              rdata[i] = byte;
            }
          }
        break;
        
        case BUS_TYPE_UART:
        break;
        
        default:
          PRINTF("Serial bus (%s): wrong bus type\n", __func__);
      }
    } else {
      return BUS_NOT_OWNED;
    }
  }
  return BUS_OK;
}
/*---------------------------------------------------------------------------*/
serial_bus_status_t
serial_arch_read(serial_dev_t *dev, uint8_t *data, uint16_t len)
{
  return serial_arch_transfer(dev, NULL, 0, data, len);
}
/*---------------------------------------------------------------------------*/
serial_bus_status_t
serial_arch_write(serial_dev_t *dev, const uint8_t *data, uint16_t len)
{
  return serial_arch_transfer(dev, data, len, NULL, 0);
}
/*---------------------------------------------------------------------------*/
void
serial_dev_uart_set_input_handler(serial_dev_t *dev, uint8_t (*handler)(unsigned char c))
{
  dev->bus->config.uart_input_handler = handler;
  /* Clear Framing err, parity err and RX overflow err flags */
  USART_IntClear(dev->bus->config.SPI_UART_USARTx, USART_IF_FERR | USART_IF_PERR | USART_IF_RXOF);
  /* clear any pending NVIC interrupts */
  clear_NVIC_pending(dev->bus->config.SPI_UART_USARTx, RX_NVIC);
  /* Enable RX data available, framing, parity and overflow interrupt flags */
  USART_IntEnable(dev->bus->config.SPI_UART_USARTx, USART_IF_RXDATAV | USART_IF_FERR | USART_IF_PERR | USART_IF_RXOF);
  /* Enable RX NVIC interrupt request */
  enable_NVIC_interrupt(dev->bus->config.SPI_UART_USARTx, RX_NVIC);
}
/*---------------------------------------------------------------------------*/
