#ifndef _WATCHDOG_ARCH_H_
#define _WATCHDOG_ARCH_H_

#define WDOG_TIME_IS_DEFINED 1
typedef enum wdog_time {
  wdog_time_9_ms = 0x0,
  wdog_time_17_ms = 0x1,
  wdog_time_33_ms = 0x2,
  wdog_time_65_ms = 0x3,
  wdog_time_129_ms = 0x4,
  wdog_time_257_ms = 0x5,
  wdog_time_513_ms = 0x6,
  wdog_time_1s025 = 0x7,
  wdog_time_2s049 = 0x8,
  wdog_time_4s097 = 0x9,
  wdog_time_8s193 = 0xA,
  wdog_time_16s385 = 0xB,
  wdog_time_32s769 = 0xC,
  wdog_time_65_s537 = 0xD,
  wdog_time_131s073 = 0xE,
  wdog_time_262s145 = 0xF,
} wdog_time_t;

#endif /* _WATCHDOG_ARCH_H_ */
