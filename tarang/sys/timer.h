#ifndef _TIMER_H_
#define _TIMER_H_
#include "clock.h"

typedef struct ttimer {
  clock_time_t start_time;
  clock_time_t interval;
} ttimer_t;

void timer_set(ttimer_t *timer, clock_time_t interval);  /* set the timer for given interval */
void timer_reset(ttimer_t *timer);                       /* reset the timer start time with set interval */
void timer_reload(ttimer_t *timer);                      /* update the start time of the timer */
bool timer_timedout(ttimer_t *timer);                    /* returns true of the timer has expired */

#endif /* _TIMER_H_ */