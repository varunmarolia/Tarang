#include "pwm-dev.h"

/*---------------------------------------------------------------------------*/
void
pwm_dev_init(pwm_dev_t *dev)
{
  if(dev != NULL && dev->config != NULL) {
    pwm_arch_init(dev);
  }
  pwm_arch_enable_device(dev, PWM_DEVICE_ENABLE);
}
/*---------------------------------------------------------------------------*/
void
pwm_dev_set_duty_cycle(pwm_dev_t *dev)
{
  if(dev != NULL && dev->config != NULL) {
    pwm_arch_set_duty_cycle(dev);
  }
}
/*---------------------------------------------------------------------------*/
void
pwm_dev_reset(pwm_dev_t *dev) 
{
  pwm_arch_enable_device(dev, PWM_DEVICE_DISABLE);
  if(dev != NULL && dev->config != NULL) {
    pwm_arch_reset(dev);
  }
}
/*---------------------------------------------------------------------------*/
