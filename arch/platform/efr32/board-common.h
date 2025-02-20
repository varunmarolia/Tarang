#ifndef _BOARD_COMMON_H_
#define _BOARD_COMMON_H_
#include <stdint.h>
#include "em_gpio.h"

#ifndef LED_ON
#define LED_ON  1 /* active low default setup */
#endif  /* LED_ON */
#ifndef LED_OFF
#define LED_OFF 0
#endif  /* LED_OFF */

void led_set(GPIO_Port_TypeDef port, uint32_t pin, uint8_t on_off);
void led_blink(GPIO_Port_TypeDef port, uint32_t pin, uint8_t times, uint32_t delay_ms);
#endif /* _BOARD_COMMON_H_ */
