#ifndef _COMMON_ARCH_H_
#define _COMMON_ARCH_H_
#include <em_gpio.h>

typedef enum enable_logic {
  ENABLE_ACTIVE_LOW = 0,
  ENABLE_ACTIVE_HIGH = 1
} enable_logic_t;

typedef struct gpio_config {
  const GPIO_Port_TypeDef port;   /* port name */
  const uint8_t pin;              /* pin number */
  const enable_logic_t logic;     /* enable logic */
} gpio_config_t;

#endif  /* _COMMON_ARCH_H_ */
