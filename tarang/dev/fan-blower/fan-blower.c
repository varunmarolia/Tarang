/**
 * @file fan-blower.c
 * @author Varun Marolia
 * @brief This is a driver for DC fan/blowers with PWM input signal, Direction control
 *         and Techo input.
 * @todo Add support for Techo input and control system for fan/blower speed
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

#include "fan-blower.h"

#define FAN_BLOWER_DEBUG 0
#if FAN_BLOWER_DEBUG
#include <stdio.h>
#undef PRINTF
#define PRINTF(...) printf(__VA_ARGS__)
#else  /* FAN_BLOWER_DEBUG */
/**< Replace printf with nothing */
#define PRINTF(...)
#endif /* FAN_BLOWER_DEBUG */
/*---------------------------------------------------------------------------*/
void 
fan_blower_init(fan_blower_t *fb)
{
  if(fb != NULL && fb->pwm_dev != NULL) {
    /* Initialize the pwm unit and enable the device */
    if(pwm_dev_init(fb->pwm_dev) != PWM_STATUS_OK) {
      PRINTF("fan-blower: Could not initialize the pwm device !!!\n");
    }
  } else {
    PRINTF("fan-blower: Null pointer input!!!\n");
  }
}
/*---------------------------------------------------------------------------*/
void 
fan_blower_set_rpm(fan_blower_t *fb, uint32_t rpm, uint8_t dir)
{
  uint32_t new_duty_cycle_100x = fb->min_pwm * 100;
  if(fb != NULL && fb->pwm_dev != NULL) {
    if(rpm < fb->min_rpm && rpm > 0) {
      rpm = fb->min_rpm;
    }
    if( rpm > fb->max_rpm) {
      rpm = fb->max_rpm;
    }
    switch (fb->bidir) {
      case FAN_BLOWER_DIR_BIDIRECTIONAL_OVER_GPIO:
        if(fb->fan_blower_dir_handler !=NULL) {
          fb->fan_blower_dir_handler(dir);
        } else {
          PRINTF("Fan-blower: setup for gpio based direction control but no direction control function assigned !!!\n");
        }
      case FAN_BLOWER_DIR_SINGLE:
        new_duty_cycle_100x = 100000 / fb->max_rpm;
        new_duty_cycle_100x *= rpm;
        new_duty_cycle_100x /= 10;
        if(new_duty_cycle_100x > 10000) {
          new_duty_cycle_100x = 10000;
        }
        if(new_duty_cycle_100x < (fb->min_pwm * 100) && new_duty_cycle_100x > 0) {
          new_duty_cycle_100x = fb->min_pwm * 100;
        }
      break;
      
      case FAN_BLOWER_DIR_BIDIRECTIONAL_OVER_PWM:
        new_duty_cycle_100x = (50 - fb->min_pwm) * 1000 / fb->max_rpm;
        new_duty_cycle_100x *= rpm;
        new_duty_cycle_100x /= 10;
        if(new_duty_cycle_100x < (fb->min_pwm * 100) && new_duty_cycle_100x > 0) {
          new_duty_cycle_100x = fb->min_pwm * 100;
        }
        if(dir) {
          /* Forward direction */
          new_duty_cycle_100x = 5000 - new_duty_cycle_100x;
        } else {
          /* reverse direction */
          new_duty_cycle_100x = 5000 + new_duty_cycle_100x;
        }
      break;
      
      default:
      break;
    }
    /* apply the new duty cycle */
    pwm_dev_set_duty_cycle(fb->pwm_dev, new_duty_cycle_100x);
    /* update direction only if rpm > 0 else preserve the last direction value */
    if(rpm > 0) {
      fb->current_dir = dir;
    }
    fb->current_rpm = rpm;
    PRINTF("Fan-blower: new duty cycle:%u\n", (uint16_t)new_duty_cycle_100x);
  } else {
    PRINTF("Fan-blower: Could not find the device !!!\n");
  }
}
/*---------------------------------------------------------------------------*/
void
fan_blower_reset(fan_blower_t *fb)
{
  if(fb != NULL && fb->pwm_dev != NULL) {
    pwm_dev_reset(fb->pwm_dev);
  }
}
/*---------------------------------------------------------------------------*/
