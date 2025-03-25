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
    pwm_dev_init(fb->pwm_dev);  /* This will setup timer for given frequency in pwm mode with initial compare value */
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
    /* set the new duty cycle */
    fb->pwm_dev->duty_cycle_100x = new_duty_cycle_100x;
    /* apply the new duty cycle */
    pwm_dev_set_duty_cycle(fb->pwm_dev);
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
