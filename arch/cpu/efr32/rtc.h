#ifndef _RTC_H_
#define _RTC_H_
#include "board.h"
#include <stdint.h>

#define RTC_TICK_FREQ_HZ 32768  /* RTC counter clock frequency is 32768Hz a tick is approx 30.5 micro seconds */

void rtc_init(void);
uint64_t rtc_get_ticks(void);

#endif /* _RTC_H_ */
