#ifndef _WATCHDOG_H_
#define _WATCHDOG_H_
#include <stdint.h>
#include "watchdog-arch.h"
/* following functions must be implemented in the arch */
/**
 * @brief function should initialize the MCU regs and set up the 
 *        watchdog for given amount of time interval. Watchdog
 *        should not be enabled. use the guard function to enable it.
 * 
 * @param watchdog_time_ms watchdog timer time in milliseconds.
 */
#if defined WDOG_TIME_IS_DEFINED
void watchdog_init(wdog_time_t wdtime);

/**
 * @brief function updates the watchdog time
 * 
 * @param wdtime watchdog time 
 * @return * void 
 */
void watchdog_udpate_time(wdog_time_t wdtime);
#else
void watchdog_init(uint32_t wdtime);
void watchdog_udpate_time(wdog_time_t wdtime);
#endif /* WDOG_TIME_IS_DEFINED */
/**
 * @brief function should start/enable the watchdog timer.
 *        Make sure to call the watchdog_init function first.
 */
void watchdog_guard(void);

/** 
 * @brief function should stop/reset the watchdog timer peripheral 
 */
void watchdog_sleep(void);

/**
 * @brief function should reload the watchdog timer in order to avoid 
 *        the impending doom.
 *  
 */
void watchdog_feed(void);

/**
 * @brief Function should let the system reboot using watchdog event. 
 * 
 */
void watchdog_sic_em(void);
#endif /* _WATCHDOG_H_ */
