#include "timer.h"
/*---------------------------------------------------------------------------*/
void
timer_set(timer_t *timer, clock_time_t interval)
{
  timer->interval = interval;
  timer->start_time = clock_get_time_ms();
}
/*---------------------------------------------------------------------------*/
void
timer_reload(timer_t *timer)
{
  timer->start_time = clock_get_time_ms();
}
/*---------------------------------------------------------------------------*/
void
timer_reset(timer_t *timer)
{
  timer->start_time += timer->interval;
}
/*---------------------------------------------------------------------------*/
bool
timer_timedout(timer_t *timer)
{
  clock_time_t time_diff = (clock_get_time_ms() - timer->start_time) + 1;
  if(timer->interval < time_diff) {
    return true;
  } 
  return false;
}
/*---------------------------------------------------------------------------*/
