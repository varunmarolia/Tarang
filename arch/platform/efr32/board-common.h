#ifndef _BOARD_COMMON_H_
#define _BOARD_COMMON_H_
#include <stdint.h>
#include "em_gpio.h"
#include "board.h"
#include "adc-dev.h"

#ifndef LED_ON
#define LED_ON  1 /* active low default setup */
#endif  /* LED_ON */
#ifndef LED_OFF
#define LED_OFF 0
#endif  /* LED_OFF */

void led_set(GPIO_Port_TypeDef port, uint32_t pin, uint8_t on_off);
void led_blink(GPIO_Port_TypeDef port, uint32_t pin, uint8_t times, uint32_t delay_ms);
void print_chip_info(void);
uint32_t board_read_voltage_divider_mv(adc_dev_t *dev, uint32_t r1, uint32_t r2);
void button_reset_init(void);
#endif /* _BOARD_COMMON_H_ */
