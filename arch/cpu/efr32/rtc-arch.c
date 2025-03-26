/**
 * @file rtc-arch.c
 * @author Varun Marolia
 * @brief This file contains arch specific methods for using Real Time Clock feature of the MCU.
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
#include "rtc-arch.h"
#include "em_rtcc.h"
#include "em_cmu.h"

static volatile uint64_t tick_counter;        /* total RTC counter ticks since boot. 
                                               * A tick is approximate 30.5 microseconds (i.e. 1/32768Hz). 
                                               * overflow every 19 million years.
                                               * */
static volatile uint32_t last_ticks_value;

/*---------------------------------------------------------------------------*/
void
rtc_arch_init(void)
{
  RTCC_Init_TypeDef rtc_init;
   /* disable LFXTAL pins as recommended for analog
   * connections in reference manual.
   */
  GPIO_PinModeSet(gpioPortB, 14, gpioModeDisabled, 0);
  GPIO_PinModeSet(gpioPortB, 15, gpioModeDisabled, 0);
  /* Enable LE peripheral interface clock */
  CMU_ClockEnable(cmuClock_HFLE, true);
  CMU_LFXOInit_TypeDef lfxoInit = CMU_LFXOINIT_DEFAULT;
  /* Load the Ctune value if defined for this board */
#ifdef BOARD_LFXO_CTUNE_VALUE
  lfxoInit.ctune = BOARD_LFXO_CTUNE_VALUE;
#endif /* BOARD_LFXO_CTUNE_VALUE */
  CMU_LFXOInit(&lfxoInit);
  /* Enable LFXO as LFECLK in CMU. This will also start LFXO */
  CMU_ClockSelectSet(cmuClock_LFE, cmuSelect_LFXO);
  CMU_ClockEnable(cmuClock_RTCC, true);
  /* initialize RTC structure */
  rtc_init.enable = true;
  rtc_init.debugRun = false;
  rtc_init.precntWrapOnCCV0 = false;
  rtc_init.cntWrapOnCCV1 = false;
  rtc_init.presc = rtccCntPresc_1;
  rtc_init.prescMode = rtccCntTickPresc;
  rtc_init.enaOSCFailDetect = false;
  rtc_init.cntMode = rtccCntModeNormal;
  RTCC_Init(&rtc_init);
  /* initialize the last_ticks_value and current time */
  last_ticks_value = RTCC_CounterGet();
  tick_counter = last_ticks_value;
}
/*---------------------------------------------------------------------------*/
uint64_t
rtc_arch_get_ticks(void)
{
  return tick_counter;
}
/*---------------------------------------------------------------------------*/
