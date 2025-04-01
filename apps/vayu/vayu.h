#ifndef _VAYU_H_
#define _VAYU_H_

#define NTC_BOARD 0
#define NTC_HRV   1
#define NTC_TOTAL 2

typedef enum hrv_mode {
  HRV_MODE_OFF = 0,
  HRV_MODE_INLET = 1,
  HRV_MODE_AUTO = 2
} hrv_mode_t;

#endif /* _VAYU_H_ */
