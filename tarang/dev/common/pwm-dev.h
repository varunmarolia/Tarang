#ifndef _PWM_DEV_H_
#define _PWM_DEV_H_

#include <stdint.h>
#include <stddef.h>
#include "common-arch.h"
#include "pwm-arch.h"
typedef struct pwm_dev {
  const uint8_t cc_channel;               /* compare channel number, the device is on */
  volatile uint16_t duty_cycle_100x;      /* 100 x value of duty cycle in percentage. i.e 5124 is 51.24 % */
  const enable_logic_t pwm_active_logic;      /* Device enable logic, Active low in a 10% duty cycle means output low for 10% of the cycle */
  /* arch specific */
  const pwm_config_t *config;             /* pointer to the configuration data */
  const uint32_t gpio_loc;
  gpio_config_t *dev_enable;              /* device enable pin. Some devices have separate pins for power enable and PWM */
} pwm_dev_t;

void pwm_dev_init(pwm_dev_t *dev);
void pwm_dev_set_duty_cycle(pwm_dev_t *dev);
void pwm_dev_reset(pwm_dev_t *dev);
/* arch specific functions */
void pwm_arch_init(pwm_dev_t *dev);
void pwm_arch_reset(pwm_dev_t *dev);
void pwm_arch_set_duty_cycle(pwm_dev_t *dev);
void pwm_arch_enable_device(pwm_dev_t *dev, uint8_t on_off);
#endif /* _PWM_DEV_H_ */