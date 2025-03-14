#ifndef _FAN_BLOWER_H_
#define _FAN_BLOWER_H_
#include <stdint.h>
#include "pwm-dev.h"
#include "common-arch.h"

typedef enum fb_dir_ctrl {
  FAN_BLOWER_DIR_SINGLE = 0,                   /* No direction control */
  FAN_BLOWER_DIR_BIDIRECTIONAL_OVER_PWM = 1,   /* at 50% +/10% pwm the fan/blower is OFF from 0 to 40% duty cyle forward direciton from 60% t  */
  FAN_BLOWER_DIR_BIDIRECTIONAL_OVER_GPIO = 2   /* Direction control using GPIO */
} fb_dir_ctrl_t;

typedef struct fan_blower {
  const uint8_t pulses_per_rev;                   /* number of techo pulses per revolution */
  const fb_dir_ctrl_t bidir;                      /* set BIDIRECTIONAL if direction can be changed over PWM or GPIO */
  const uint16_t max_rpm;                         /* max rpm 65535 */
  const uint16_t min_rpm;                         /* min rpm, start rpm */
  const uint8_t min_pwm;                          /* min pwm duty cycle in percentage for min RPM. for bi-direction fan over pwm, this is the duty cycle hysteresis where the fan/blower will stay off */
  void (* fan_blower_dir_handler)(uint8_t dir);
  uint16_t current_rpm;
  uint8_t current_dir;
  /* arch specific */
  pwm_dev_t *pwm_dev;
} fan_blower_t;

typedef struct fan_blower dc_motor_t;

void fan_blower_init(fan_blower_t *fb);
void fan_blower_set_rpm(fan_blower_t *fb, uint32_t rpm, uint8_t dir);
void fan_blower_reset(fan_blower_t *fb);
#endif  /* _FAN_BLOWER_H_ */
