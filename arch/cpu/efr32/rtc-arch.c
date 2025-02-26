#include "rtc-arch.h"
#include "em_rtcc.h"
#include "em_cmu.h"

static volatile uint64_t tick_counter;        /* total RTC counter ticks since boot. 
                                               * A tick is approximate 30.5 useconds (i.e. 1/32768Hz). 
                                               * overflow every 19 milion years.
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
