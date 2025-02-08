#include "serial-arch.h"
#include "em_gpio.h"

#define DEBUG 0     /**< Set this to 1 for debug printf output */
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)      /**< Replace printf with nothing */
#endif /* DEBUG */
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
    PRINTF("I2C: lock: unsupported I2Cx: %p\n", conf->I2Cx);
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
  i2c_arch_restart_timer(dev);
  /* lock the device by setting the flag and pairing the device pointer */
  dev->bus->lock = 1;
  dev->bus->current_dev = dev;
  return BUS_OK;
}
/*---------------------------------------------------------------------------*/
static serial_bus_status_t
spi_dev_arch_init(serial_dev_t *dev)
{
  USART_TypeDef* spi_uart = dev->bus->config.SPI_USARTx;
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

  USART_InitSync_TypeDef init = USART_INITSYNC_DEFAULT;

  /* configure SPI controller */
  init.master = true;
  init.msbf     = bus_config->msb_first; 
  init.baudrate = dev->speed_hz;
  init.clockMode = bus_config->clock_mode;
  USART_InitSync(spi_uart, &init);

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

  return BUS_OK;
}
/*---------------------------------------------------------------------------*/
void 
serial_arch_chip_select(serial_dev_t *dev, uint8_t on_off)
{
  serial_bus_config_t *config = &dev->bus->config;
  GPIO_PinModeSet(config->chip_select_port, config->chip_select_pin, gpioModePushPull, 1);
  if (on_off = CHIP_SELECT_ENABLE) {
    if(config->chip_select_logic == ENABLE_ACTIVE_LOW) {
      GPIO_PinOutClear(config->chip_select_port , config->chip_select_pin);
    } else {
      GPIO_PinOutSet(config->chip_select_port , config->chip_select_pin);
    }
    if(dev->power_up_delay_ms) {
      clock_wait_ms(dev->power_up_delay_ms);
    }
  } else {
    if(config->chip_select_logic == ENABLE_ACTIVE_LOW) {
      GPIO_PinOutSet(config->chip_select_port , config->chip_select_pin);
    } else {
      GPIO_PinOutClear(config->chip_select_port , config->chip_select_pin);
    }
  }
}
/*---------------------------------------------------------------------------*/
bool
serial_arch_chip_is_selected(serial_dev_t *dev)
{
  serial_bus_config_t *config = &dev->bus->config;
  bool cs_enabled;
  
  if(config->chip_select_logic == ENABLE_ACTIVE_LOW) {
    if(!GPIO_PinOutGet(config->chip_select_port, config->chip_select_pin)) {
      cs_enabled = true;
    } else {
      cs_enabled = false;
    }
  } else {
    if(GPIO_PinOutGet(config->chip_select_port, config->chip_select_pin)) {
      cs_enabled = true;
    } else {
      cs_enabled = false;
    }
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
    /* if timer expired for the device holding the bus, unlock the bus */
    if(timer_expired(&bus_config->bus_timer)) {
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
        bus_status = spi_dev_arch_init(dev);
      break;
      case BUS_TYPE_UART:
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
        break;
        case BUS_TYPE_UART:
        break;
        default:
          PRINTF("Serial bus (%s): wrrong bus typr\n", __func__);
        break;
      } 
    } else {
      return BUS_BUSY;
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
serial_arch_read(serial_dev_t *dev, uint8_t *data, uint16_t len)
{
  return BUS_OK;
}
/*---------------------------------------------------------------------------*/
serial_bus_status_t
serial_arch_write(serial_dev_t *dev, const uint8_t *data, uint16_t len)
{
  return BUS_OK;
}
/*---------------------------------------------------------------------------*/

