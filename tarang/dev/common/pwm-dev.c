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
