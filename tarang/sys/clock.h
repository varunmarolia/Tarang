#ifndef _CLOCK_H_
#define _CLOCK_H_
#include <stdint.h>
#include <stdbool.h>

typedef uint64_t clock_time_t;        /* 64 bit 1 msecond tick counter. will take approx 584.9 million years to overflow */
#define CLOCK_TICKS_MSECOND       1000    /* SYS tick every 1ms i.e. 1/1000 */
#define CLOCK_TICKS_HALF_MSECOND  2000    /* SYS tick every 0.5ms i.e. 1/2000 */

#define CLOCK_TICKS_CONF CLOCK_TICKS_MSECOND /* current config of sys tick */

/* clock function defs that must be implemented by the plarform */
void clock_init(void);                      /* Initialize sysTick clock hardware to produce 1 msecond tick */
clock_time_t clock_get_ticks(void);         /* return system ticks since boot */
clock_time_t clock_get_time_ms(void);       /* return time in mseconds since boot*/
clock_time_t clock_get_seconds(void);       /* returns seconds past since last boot */
void clock_wait_ms(clock_time_t time_ms);   /* busy wait for given amount of miliseconds */
void clock_wait_us(uint32_t time_us);       /* buys wait for fiven amount fo microseconds */
#endif /* _CLOCK_H_ */
