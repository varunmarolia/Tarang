#ifndef _BOARD_H_
#define _BOARD_H_
#include "board-akashvani1.h"

#ifndef BOARD_NAME
#define BOARD_NAME "Vayu_R1A"
#endif /* BOARD_NAME */

#define LED_SYS_GREEN_PORT      gpioPortD
#define LED_SYS_GREEN_PIN       11
#define LED_SYS_YELLOW_PORT     gpioPortD
#define LED_SYS_YELLOW_PIN      12

#define LED_MODE_GREEN_PORT     gpioPortF
#define LED_MODE_GREEN_PIN      3
#define LED_MODE_YELLOW_PORT    gpioPortF
#define LED_MODE_YELLOW_PIN     2

#define USART_BASE      USART1
#define UART_TX_PORT    gpioPortB
#define UART_TX_PIN     12
#define UART_TX_LOC     _USART_ROUTELOC0_RXLOC_LOC6

#define UART_RX_PORT    gpioPortB
#define UART_RX_PIN     11
#define UART_RX_LOC     _USART_ROUTELOC0_TXLOC_LOC6

#define UART_RTS_PORT   gpioPortC
#define UART_RTS_PIN    6
#define UART_RTS_LOC    _USART_ROUTELOC1_CTSLOC_LOC7

#define UART_CTS_PORT   gpioPortC
#define UART_CTS_PIN    11
#define UART_CTS_LOC    _USART_ROUTELOC1_RTSLOC_LOC11

/* Use above defined uart for debugging purposes */
#define DEBUG_USART_BASE            USART_BASE
#define DEBUG_UART_TX_LOC           UART_RX_LOC
#define DEBUG_UART_RX_LOC           UART_TX_LOC
#define DEBUG_UART_RTS_LOC          UART_RTS_LOC
#define DEBUG_UART_CTS_LOC          UART_CTS_LOC

#define BOARD_SUPPLY_TEMP_ADC_PORT    gpioPortD
#define BOARD_SUPPLY_TEMP_ADC_PIN     13
#define BOARD_SUPPLY_TEMP_ADC_INPUT   adcPosSelAPORT4XCH5

#define BOARD_SUPPLY_SENSE_ENABEL_BAR_PORT    gpioPortD
#define BOARD_SUPPLY_SENSE_ENABEL_BAR_PIN     15
#define BOARD_NTC_SENSE_ENABLE_BAR_PORT       gpioPortD
#define BOARD_NTC_SENSE_ENABLE_BAR_PIN        14

#define HRV_HEATER_ENABLE_BAR_PORT          gpioPortD
#define HRV_HEATER_ENABLE_BAR_PIN           10
#define FAN_ENABLE_BAR_PORT                 gpioPortA
#define FAN_ENABLE_BAR_PIN                  0
#define FAN_PWM_PORT                        gpioPortA
#define FAN_PWM_PIN                         1
#define FAN_TECHO_PORT                      gpioPortA
#define FAN_TECHO_PIN                       2
#define HRV_NTC_ADC_PORT                    gpioPortA
#define HRV_NTC_ADC_PIN                     3
#define HRV_NIT_ADC_INPUT                   adcPosSelAPORT4XCH11

#define I2C_BUS_DATA_PORT       gpioPortA
#define I2C_BUS_DATA_PIN        5
#define I2C_BUS_DATA_LOC        _I2C_ROUTELOC0_SDALOC_LOC5
#define I2C_BUS_CLOCK_PORT      gpioPortA
#define I2C_BUS_CLOCK_PIN       4
#define I2C_BUS_CLOCK_LOC       _I2C_ROUTELOC0_SCLLOC_LOC3
#define SHT4X_I2C_BUS           i2c_bus_0

#define MODE_PUSH_BUTTON_PORT   gpioPortC
#define MODE_PUSH_BUTTON_PIN    10
#define RESET_PUSH_BUTTON_PORT  gpioPortB
#define RESET_PUSH_BUTTON_PIN   13

void board_init(void);
#endif /* _BOARD_H_ */
