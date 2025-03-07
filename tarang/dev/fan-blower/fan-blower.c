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
    pwm_dev_init(fb->pwm_dev);  /* This will setup timer for given frequency in pwm mode with intitial compare value */
  } else {
    PRINTF("fan-blower: Null pointer input!!!\n");
  }
}
/*---------------------------------------------------------------------------*/
void 
fan_blower_set_rpm(fan_blower_t *fb, uint32_t rpm, fb_dir_t dir)
{
  uint32_t new_duty_cycle_100x;
  if(fb != NULL && fb->pwm_dev != NULL) {
    if(rpm < fb->min_rpm) {
      rpm = fb->min_rpm;
    }
    if( rpm > fb->max_rpm) {
      rpm = fb->max_rpm;
    }
    switch (fb->bidir) {
      case FAN_BLOWER_DIR_BIDIRECTIONAL_OVER_GPIO:
        if(fb->dir != NULL) {
          //set direction
        }
      case FAN_BLOWER_DIR_SINGLE:
        new_duty_cycle_100x = 100000 / fb->max_rpm;
        new_duty_cycle_100x *= rpm;
        new_duty_cycle_100x /= 10;
        if(new_duty_cycle_100x > 10000) {
          new_duty_cycle_100x = 10000;
        }
        if(new_duty_cycle_100x < (fb->min_pwm * 100)) {
          new_duty_cycle_100x = fb->min_pwm * 100;
        }
        /* set the new duty cycle */
        fb->pwm_dev->duty_cycle_100x = new_duty_cycle_100x;
        /* apply the new duty cycle */
        pwm_dev_set_duty_cycle(fb->pwm_dev);
      break;
      
      case FAN_BLOWER_DIR_BIDIRECTIONAL_OVER_PWM:
      break;
      
      default:
      break;
    }
  }
}
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
