#ifndef _FAN_BLOWER_H_
#define _FAN_BLOWER_H_
#include <stdint.h>
#include "pwm-dev.h"
#include "common-arch.h"

typedef enum fb_dir {
  FAN_BLOWER_DIR_FORWARD = 0,
  FAN_BLOWER_DIR_REVERSE = 1
} fb_dir_t;

typedef enum fb_dir_ctrl {
  FAN_BLOWER_DIR_SINGLE = 0,                   /* No direction control */
  FAN_BLOWER_DIR_BIDIRECTIONAL_OVER_PWM = 1,   /* at 50% +/10% pwm the fan/blower is OFF from 0 to 40% duty cyle forward direciton from 60% t  */
  FAN_BLOWER_DIR_BIDIRECTIONAL_OVER_GPIO = 2   /* Direction control using GPIO */
} fb_dir_ctrl_t;

typedef struct fan_blower {
  const uint8_t pulses_per_rev;         /* number of techo pulses per revolution */
  const fb_dir_ctrl_t bidir;            /* set BIDIRECTIONAL if direction can be changed over PWM or GPIO */
  const uint16_t max_rpm;               /* max rpm 65535 */
  const uint16_t min_rpm;               /* min rpm, start rpm */
  const uint8_t min_pwm;                /* min pwm for min RPM. For bidireciton fan over PWM, there is a duty cycle range where the fan/blower will stay off */
  /* arch specific */
  gpio_config_t *dir;                   /* direction gpio pin. Set to NULL if there is no direction control or direciton control is over PWM */
  pwm_dev_t *pwm_dev;
} fan_blower_t;

typedef struct fan_blower dc_motor_t;

void fan_blower_init(fan_blower_t *fb);
void fan_blower_set_rpm(fan_blower_t *fb, uint32_t rpm, fb_dir_t dir);
#endif  /* _FAN_BLOWER_H_ */
