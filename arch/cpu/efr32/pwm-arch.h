#ifndef _PWM_ARCH_H_
#define _PWM_ARCH_H_
#include <stdint.h>
#include <em_timer.h>

#define PWM_DEVICE_DISABLE  0
#define PWM_DEVICE_ENABLE   1
typedef struct pwm_config {
  const uint32_t freq_hz;
  /* arch specific */
  const TIMER_TypeDef *timer_per;
} pwm_config_t;

#endif /* _PWM_ARCH_H_ */
