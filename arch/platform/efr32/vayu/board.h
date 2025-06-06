#ifndef _BOARD_H_
#define _BOARD_H_
#include "board-akashvani1.h"
#include "serial-dev.h"
#include "adc-dev.h"
#include "common-arch.h"
#include "sht4x.h"
#include "pwm-dev.h"
#include "gpio.h"

#ifndef BOARD_NAME
#define BOARD_NAME "Vayu_R1A"
#endif /* BOARD_NAME */

#define LED_SYS_GREEN_PORT      GPIO_PORT_F
#define LED_SYS_GREEN_PIN       3
#define LED_SYS_YELLOW_PORT     GPIO_PORT_F
#define LED_SYS_YELLOW_PIN      2

#define LED_MODE_GREEN_PORT     GPIO_PORT_D
#define LED_MODE_GREEN_PIN      11
#define LED_MODE_YELLOW_PORT    GPIO_PORT_D
#define LED_MODE_YELLOW_PIN     12

#define UART_USART            USART0 /* only USART0 or USART1 can be used for UART functionality */
#define UART_MCU_RX_PORT      GPIO_PORT_B
#define UART_MCU_RX_PIN       12
#define UART_MCU_RX_LOC       _USART_ROUTELOC0_TXLOC_LOC6

#define UART_MCU_TX_PORT      GPIO_PORT_B
#define UART_MCU_TX_PIN       11
#define UART_MCU_TX_LOC       _USART_ROUTELOC0_RXLOC_LOC6

#define UART_GENERIC_DEV        guart_dev

#define SWO_DEBUG_LOC                 GPIO_ROUTELOC0_SWVLOC_LOC3    /* PC11 */

#define BOARD_SUPPLY_ADC_PORT         GPIO_PORT_D
#define BOARD_SUPPLY_ADC_PIN          13
#define BOARD_SUPPLY_ADC_INPUT        adcPosSelAPORT4XCH5
#define BOARD_SUPPLY_R1_OHMS          47000                 /* Voltage divider R1 47K */
#define BOARD_SUPPLY_R2_OHMS          100000                /* Voltage divider R2 100K */

#define BOARD_NTC_ADC_PORT            GPIO_PORT_D
#define BOARD_NTC_ADC_PIN             14
#define BOARD_NTC_ADC_INPUT           adcPosSelAPORT3XCH6

#define FAN_12V_SUPPLY_ADC_PORT       GPIO_PORT_D
#define FAN_12V_SUPPLY_ADC_PIN        15
#define FAN_12V_SUPPLY_ADC_INPUT      adcPosSelAPORT4XCH7
#define FAN_12V_SUPPLY_R1_OHMS        47000                 /* Voltage divider R1 47K */
#define FAN_12V_SUPPLY_R2_OHMS        10000                 /* Voltage divider R2 10K */ 

#define BOARD_NTC_ADC_DEV            ntc_board_adc
#define BOARD_SUPPLY_ADC_DEV         supply_board_adc
#define FAN_12V_ADC_DEV              fan_12v_adc

#define HA_HEATER_ENABLE_PORT        GPIO_PORT_D
#define HA_HEATER_ENABLE_PIN         10
#define HA_HEATER_ROUTE_LOC          _TIMER_ROUTELOC0_CC1LOC_LOC17  /* Timer 0 CC1 PD10 */

#define FAN_ENABLE_PORT              GPIO_PORT_A
#define FAN_ENABLE_PIN               0
#define FAN_PWM_PORT                 GPIO_PORT_A
#define FAN_PWM_PIN                  1
#define FAN_PWM_ROUTE_LOC            _TIMER_ROUTELOC0_CC0LOC_LOC1 /* Timer 0 CC0 PA1 */
#define FAN_TECHO_PORT               GPIO_PORT_A
#define FAN_TECHO_PIN                2
#define FAN_PWM_DEV                  fan_dev
#define FAN_DIR_FORWARD              1          /* exhaust */
#define FAN_DIR_REVERSE              0          /* inlet */

#define HA_NTC_ADC_PORT              GPIO_PORT_A
#define HA_NTC_ADC_PIN               3
#define HA_NTC_ADC_INPUT             adcPosSelAPORT4XCH11
#define HA_NTC_ADC_DEV               ntc_ha_adc
#define HA_HEATER_DEV                ha_heater_dev

#define BOARD_ADC_PER                ADC0
#define BOARD_ADC_REF_mVDD           3000  /* 3 Volts onboard regulator output of Akashvani */

#define I2C_BUS_DATA_PORT             GPIO_PORT_A
#define I2C_BUS_DATA_PIN              5
#define I2C_BUS_DATA_LOC              _I2C_ROUTELOC0_SDALOC_LOC5
#define I2C_BUS_CLOCK_PORT            GPIO_PORT_A
#define I2C_BUS_CLOCK_PIN             4
#define I2C_BUS_CLOCK_LOC             _I2C_ROUTELOC0_SCLLOC_LOC3
#define SHT4X_DEV                     sht4x_dev

#define MODE_PUSH_BUTTON_PORT         GPIO_PORT_C
#define MODE_PUSH_BUTTON_PIN          10
#define MODE_BUTTON                   mode_button
#define RESET_PUSH_BUTTON_PORT        GPIO_PORT_B
#define RESET_PUSH_BUTTON_PIN         13
#define RESET_BUTTON                  reset_button

/* low level board device structures */
extern adc_dev_t HA_NTC_ADC_DEV;
extern adc_dev_t BOARD_NTC_ADC_DEV;
extern adc_dev_t BOARD_SUPPLY_ADC_DEV;
extern adc_dev_t FAN_12V_ADC_DEV;
extern serial_dev_t SHT4X_DEV;
extern pwm_dev_t FAN_PWM_DEV;
extern pwm_dev_t HA_HEATER_DEV;
extern serial_dev_t UART_GENERIC_DEV;
extern gpio_interrupt_t RESET_BUTTON;
extern gpio_interrupt_t MODE_BUTTON;

/* common board and platform functions */
void board_init(void);
uint8_t app_init(void);
void app_poll(void);

#endif /* _BOARD_H_ */
