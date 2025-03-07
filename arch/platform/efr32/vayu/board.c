#include "board.h"
#include "serial-dev.h"
#include "adc-dev.h"
#include "common-arch.h"
#include "sht4x.h"
#include "pwm-dev.h"
/*---------------------------------------------------------------------------*/
serial_bus_t i2c_bus_0 = {
  .lock = false,
  .current_dev = NULL,
  .config = {
    .data_in_loc = I2C_BUS_DATA_LOC,
    .data_out_loc = I2C_BUS_DATA_LOC,
    .clk_loc = I2C_BUS_CLOCK_LOC,
    .type = BUS_TYPE_I2C,
    .I2Cx = I2C0,
    .SPI_UART_USARTx = NULL
  }
};
serial_dev_t sht4x_dev = {
  .bus          = &i2c_bus_0,
  .speed_hz     = SHT4X_I2C_SPEED,
  .address      = SHT4X_I2C_DEFAULT_ADDRESS,
  .timeout_ms   = 200,
  .power_up_delay_ms = SHT4X_POWER_UP_TIME_MS,
  .cs_config    = NULL
};  /**< sht4x temp-humidity sensor is an i2c device */
/*---------------------------------------------------------------------------*/
serial_bus_t generic_uart_bus = {
  .lock = false,
  .current_dev = NULL,
  .config = {
    .data_in_loc = UART_MCU_RX_LOC,
    .data_out_loc = UART_MCU_TX_LOC,
    .cts_loc = UART_MCU_CTS_LOC,
    .rts_loc = UART_MCU_RTS_LOC,
    .parity_mode = USART_FRAME_PARITY_NONE,
    .stop_bits = USART_FRAME_STOPBITS_ONE,
    .uart_mode = UART_MODE_TX_RX,
    .input_handler = NULL,
    .output_handler = NULL,
    .type = BUS_TYPE_UART,
    .I2Cx = NULL,
    .SPI_UART_USARTx = UART_USART
  }
};
/*---------------------------------------------------------------------------*/
/* external NTC 100K Hisense 3950K HRV sensor */
adc_config_t ntc_hrv_config = {
  .adc_peripheral = BOARD_ADC_PER,
  .adc_ref_mv = adcRefVDD,                /* if selected vdd, Vdd here is 3 volts */
  .pos_input = HA_NTC_ADC_INPUT,
  .neg_input = adcNegSelVSS
};
adc_dev_t ntc_ha_adc = {
  .adc_avg_samples = 10,
  .power_up_delay_ms = 0,
  .adc_config = &ntc_hrv_config,
  .adc_dev_enable = NULL
};
/*---------------------------------------------------------------------------*/
/* on board NTC 100K sensor */
gpio_config_t ntc_board_enable_config = {
  .port = BOARD_NTC_SENSE_ENABLE_BAR_PORT,
  .pin = BOARD_NTC_SENSE_ENABLE_BAR_PIN,
  .logic = ENABLE_ACTIVE_LOW
};
adc_config_t ntc_supply_board_config = {
  .adc_peripheral = BOARD_ADC_PER,
  .adc_ref_mv = adcRefVDD,                /* if selected vdd, Vdd here is 3 volts */
  .pos_input = BOARD_SUPPLY_TEMP_ADC_INPUT,
  .neg_input = adcNegSelVSS
};
adc_dev_t ntc_board_adc = {
  .adc_avg_samples = 10,
  .power_up_delay_ms = 1,
  .adc_config = &ntc_supply_board_config,
  .adc_dev_enable = &ntc_board_enable_config
};
/*---------------------------------------------------------------------------*/
/* on board supply voltage (3.3 Volt) meter */
gpio_config_t supply_board_enable_config = {
  .port = BOARD_SUPPLY_SENSE_ENABLE_BAR_PORT,
  .pin = BOARD_SUPPLY_SENSE_ENABLE_BAR_PIN,
  .logic = ENABLE_ACTIVE_LOW
};
adc_dev_t supply_board_adc = {
  .adc_avg_samples = 10,
  .power_up_delay_ms = 1,
  .adc_config = &ntc_supply_board_config,
  .adc_dev_enable = &supply_board_enable_config
};
/*---------------------------------------------------------------------------*/
pwm_config_t fan_config = {
  .freq_hz = 25000,
  .timer_per = TIMER0
};
gpio_config_t fan_enable_config = {
  .port = FAN_ENABLE_BAR_PORT,
  .pin = FAN_ENABLE_BAR_PIN,
  .logic = ENABLE_ACTIVE_LOW
};
pwm_dev_t fan_dev = {
  .pwm_active_logic = ENABLE_ACTIVE_LOW,  /* ignored for bidirectional fan type */
  .cc_channel = 0,
  .gpio_loc = FAN_PWM_ROUTE_LOC,
  .duty_cycle_100x = 5000,                /* 50% duty cycle keeps the bidirectional fan OFF */
  .config = &fan_config,
  .dev_enable = &fan_enable_config
};
/*---------------------------------------------------------------------------*/
pwm_config_t ha_heater_config = {
  .freq_hz = 25000,
  .timer_per = TIMER0
};
pwm_dev_t ha_heater_dev = {
  .pwm_active_logic = ENABLE_ACTIVE_LOW,
  .cc_channel = 1,
  .gpio_loc = HA_HEATER_ROUTE_LOC,
  .duty_cycle_100x = 0,
  .config = &ha_heater_config,
  .dev_enable = NULL
};
/*---------------------------------------------------------------------------*/
void
board_init(void) {
  /* Initialize the RF module. This will select XTALs */
  akashvani1_module_init();
  /* Enable clock for GPIO */
  CMU_ClockEnable(cmuClock_GPIO, true);
  /* Configure LED_PORT pin LED_PIN (User LED) as push/pull outputs */
  GPIO_PinModeSet(LED_SYS_GREEN_PORT,         /* Port */
                  LED_SYS_GREEN_PIN,          /* Pin */
                  gpioModePushPull,           /* Mode */
                  1 );                        /* Output value */
  GPIO_PinModeSet(LED_SYS_YELLOW_PORT,        /* Port */
                  LED_SYS_YELLOW_PIN,         /* Pin */
                  gpioModePushPull,           /* Mode */
                  1 );                        /* Output value */
  GPIO_PinModeSet(LED_MODE_GREEN_PORT,        /* Port */
                  LED_MODE_GREEN_PIN,         /* Pin */
                  gpioModePushPull,           /* Mode */
                  1 );                        /* Output value */
  GPIO_PinModeSet(LED_MODE_YELLOW_PORT,       /* Port */
                  LED_MODE_YELLOW_PIN,        /* Pin */
                  gpioModePushPull,           /* Mode */
                  1 );                        /* Output value */
  /* Initialize buttons */
  GPIO_PinModeSet(RESET_PUSH_BUTTON_PORT, 
                  RESET_PUSH_BUTTON_PIN, 
                  gpioModeInput, 
                  1);
  GPIO_PinModeSet(MODE_PUSH_BUTTON_PORT,
                  MODE_PUSH_BUTTON_PIN,
                  gpioModeInput,
                  1);
  /* Disable ADC input pins for board supply and temperature measurement */
  GPIO_PinModeSet(BOARD_NTC_SENSE_ENABLE_BAR_PORT, 
                  BOARD_NTC_SENSE_ENABLE_BAR_PIN,
                  gpioModeDisabled, 
                  0);
  GPIO_PinModeSet(BOARD_SUPPLY_SENSE_ENABLE_BAR_PORT, 
                  BOARD_SUPPLY_SENSE_ENABLE_BAR_PIN,
                  gpioModeDisabled, 
                  0);
}
