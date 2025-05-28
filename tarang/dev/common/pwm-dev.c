/**
 * @file pwm-dev.c
 * @author Varun Marolia
 * @brief This file contains methods for using PWM devices.
 * 
 * @copyright Copyright (c) 2025 Varun Marolia
 *   MIT License
 *   Permission is hereby granted, free of charge, to any person obtaining a copy
 *   of this software and associated documentation files (the "Software"), to deal
 *   in the Software without restriction, including without limitation the rights
 *   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *   copies of the Software, and to permit persons to whom the Software is
 *   furnished to do so, subject to the following conditions:**
 *
 *   The above copyright notice and this permission notice shall be included in all
 *   copies or substantial portions of the Software.
 *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *   SOFTWARE.
 * 
 */

#include "pwm-dev.h"

/*---------------------------------------------------------------------------*/
pwm_status_t
pwm_dev_init(pwm_dev_t *dev)
{
  pwm_status_t status = PWM_STATUS_OK;
  if(dev != NULL && dev->config != NULL) {
    status = pwm_arch_init(dev);
  } else {
    return PWM_STATUS_INVALID_PARAM;
  }
  if(status == PWM_STATUS_OK) {
    status = pwm_arch_enable_device(dev, PWM_DEVICE_ENABLE);
  }
  return status;
}
/*---------------------------------------------------------------------------*/
pwm_status_t
pwm_dev_set_duty_cycle(pwm_dev_t *dev, uint16_t duty_cycle_100x)
{
  if(dev != NULL && dev->config != NULL) {
    if(duty_cycle_100x > 10000) {
      duty_cycle_100x = 10000; /* duty cycle cannot be more than 100% */
    }
    /* only update the pwm buffer value if it is different than the current value */
    if(dev->duty_cycle_100x != duty_cycle_100x) {
      dev->duty_cycle_100x = duty_cycle_100x; /* set the new duty cycle */
      return pwm_arch_set_duty_cycle(dev);
    } else {
      return PWM_STATUS_OK;
    }
  } else {
    return PWM_STATUS_INVALID_PARAM;
  }
  return PWM_STATUS_OK;
}
/*---------------------------------------------------------------------------*/
pwm_status_t
pwm_dev_reset(pwm_dev_t *dev) 
{
  pwm_status_t status = PWM_STATUS_OK;

  if(dev != NULL && dev->config != NULL) {
    status = pwm_arch_enable_device(dev, PWM_DEVICE_DISABLE);
    if(status != PWM_STATUS_OK) {
      return status; /* if disabling the device fails, return error */
    }
    /* reset the pwm arch for the given device */
    status = pwm_arch_reset(dev);
  } else {
    return PWM_STATUS_INVALID_PARAM;
  }
  return status;
}
/*---------------------------------------------------------------------------*/
