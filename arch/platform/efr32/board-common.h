#ifndef _BOARD_COMMON_H_
#define _BOARD_COMMON_H_
#include <stdint.h>
#include "em_gpio.h"
#include "board.h"
#include "adc-dev.h"

#ifndef LED_SYS_ON
#define LED_SYS_ON  1 /* active low default setup */
#endif  /* LED_SYS_ON */
#ifndef LED_SYS_OFF
#define LED_SYS_OFF 0
#endif  /* LED_SYS_OFF */

void led_sys_set(gpio_port_t port, uint8_t pin, uint8_t on_off);
void led_sys_blink(gpio_port_t port, uint8_t pin, uint8_t times, uint32_t delay_ms);
void print_chip_info(void);
uint32_t board_read_voltage_divider_mv(adc_dev_t *dev, uint32_t r1, uint32_t r2);
#endif /* _BOARD_COMMON_H_ */
