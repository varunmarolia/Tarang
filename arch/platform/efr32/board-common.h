#ifndef _BOARD_COMMON_H_
#define _BOARD_COMMON_H_
#include <stdint.h>
#include "em_gpio.h"
#include "board.h"

#ifndef LED_ON
#define LED_ON  1 /* active low default setup */
#endif  /* LED_ON */
#ifndef LED_OFF
#define LED_OFF 0
#endif  /* LED_OFF */

void led_set(GPIO_Port_TypeDef port, uint32_t pin, uint8_t on_off);
void led_blink(GPIO_Port_TypeDef port, uint32_t pin, uint8_t times, uint32_t delay_ms);
void print_chip_info(void);
#ifdef BOARD_SUPPLY_TEMP_ADC_INPUT
int32_t board_sensors_get_temp_mcelsius(void);
uint32_t board_sensors_get_mvoltage(void);
void button_reset_init(void);
#endif /* BOARD_SUPPLY_TEMP_ADC_INPUT */
#endif /* _BOARD_COMMON_H_ */
