/**
 * @file watchdog-arch.c
 * @author Varun Marolia
 * @brief This file contains arch specific methods for watchdog
 *        feature of the MCU.
 * 
 * @copyright Copyright (c) 2025 Varun Marolia
 *   MIT License
 *   Permission is hereby granted, free of charge, to any person obtaining a copy
 *   of this software and associated documentation files (the "Software"), to deal
 *   in the Software without restriction, including without limitation the rights
 *   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *   copies of the Software, and to permit persons to whom the Software is
 *   furnished to do so, subject to the following conditions:**
 *
 *   The above copyright notice and this permission notice shall be included in all
 *   copies or substantial portions of the Software.
 *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *   SOFTWARE.
 * 
 */
#include "watchdog-arch.h"
#include "watchdog.h"
#include <em_wdog.h>

#if defined DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif  /* defined DEBUG */

/*---------------------------------------------------------------------------*/
static void __attribute__((used))
wdog_isr_handler(uint32_t sp)
{
#if DEBUG
  uint32_t irq_flags;
  uint32_t pc;
  pc = *(uint32_t *)(sp + 24);  /* Address where wdog IRQ occured is on the stack */
  irq_flags = WDOGn_IntGet(WDOG0);
  if(irq_flags & WDOG_IF_WARN) {
    PRINTF("WDOG: Warning @ 0x%08lx\n", pc);
  }
  if(irq_flags & WDOG_IF_TOUT) {
    PRINTF("WDOG: Timeout @ 0x%08lx\n", pc);
  }
#endif  /* DEBUG */
  /* clear irq */
  WDOGn_IntClear(WDOG0, WDOG_IF_TOUT | WDOG_IEN_WARN);  /* clear any pending interrupts */
}
/*---------------------------------------------------------------------------*/
void __attribute__((naked))
WDOG0_IRQHandler()
{
  /* call wdog_isr_handler with sp as parameter */
  __ASM volatile ("mov r0, sp\n\t"
                "b   wdog_isr_handler");
}
/*---------------------------------------------------------------------------*/
void
watchdog_udpate_time(wdog_time_t wdtime)
{
  WDOG_Init_TypeDef wdog_init = WDOG_INIT_DEFAULT;
  
  WDOGn_Enable(WDOG0, false);
  wdog_init.enable = false;
  wdog_init.debugRun = false;           /* don't run during degbugging */
  wdog_init.em2Run = false;             /* don't run in EM2 and EM3 LP modes */
  wdog_init.em3Run = false;
  wdog_init.em4Block = false;           /* allow entering shutoff/hibernet mode */
  wdog_init.swoscBlock = false;         /* don't disable Oscillator */
  wdog_init.lock = false;               /* Make it possible to reconfigure the wdog */
  wdog_init.clkSel = wdogClkSelULFRCO;  /* Use internal ultra low freq 1KHz clock source */
  wdog_init.perSel = wdtime;
  wdog_init.resetDisable = false;
  wdog_init.warnSel = wdogWarnTime75pct;/* warning at 75% */
  WDOGn_Init(WDOG0, &wdog_init);        /* Init watchdog this will not start it */
  WDOGn_Enable(WDOG0, true);
}
/*---------------------------------------------------------------------------*/
void
watchdog_init(wdog_time_t wdtime)
{
  WDOG_Init_TypeDef wdog_init = WDOG_INIT_DEFAULT;
  
  wdog_init.enable = false;
  wdog_init.debugRun = false;           /* don't run during degbugging */
  wdog_init.em2Run = false;             /* don't run in EM2 and EM3 LP modes */
  wdog_init.em3Run = false;
  wdog_init.em4Block = false;           /* allow entering shutoff/hibernet mode */
  wdog_init.swoscBlock = false;         /* don't disable Oscillator */
  wdog_init.lock = false;               /* Make it possible to reconfigure the wdog */
  wdog_init.clkSel = wdogClkSelULFRCO;  /* Use internal ultra low freq 1KHz clock source */
  wdog_init.perSel = wdtime;
  wdog_init.resetDisable = false;
  wdog_init.warnSel = wdogWarnTime75pct;/* warning at 75% */
  WDOGn_Init(WDOG0, &wdog_init);        /* Init watchdog this will not start it */
  WDOGn_IntEnable(WDOG0, WDOG_IF_TOUT | WDOG_IEN_WARN); /* Enable interrupts */
  WDOGn_IntClear(WDOG0, WDOG_IF_TOUT | WDOG_IEN_WARN);  /* clear any pending interrupts */
  NVIC_ClearPendingIRQ(WDOG0_IRQn);
  NVIC_EnableIRQ(WDOG0_IRQn);
}
/*---------------------------------------------------------------------------*/
void
watchdog_guard(void)
{
  WDOGn_Enable(WDOG0, true);
}
/*---------------------------------------------------------------------------*/
void
watchdog_sleep(void)
{
  WDOGn_Enable(WDOG0, false);
}
/*---------------------------------------------------------------------------*/
void
watchdog_feed(void)
{
  WDOGn_Feed(WDOG0);
}
/*---------------------------------------------------------------------------*/
void 
watchdog_sic_em(void)
{
  WDOGn_Enable(WDOG0, false);
  watchdog_init(wdog_time_9_ms);
  WDOGn_Enable(WDOG0, true);
  /* wait for reboot */
  while(1) {
    __ASM volatile ("nop");
  }
}
/*---------------------------------------------------------------------------*/


