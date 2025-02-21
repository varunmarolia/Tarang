#include "em_device.h"
#include "em_gpio.h"
#include "em_cmu.h"
#include "board.h"
#include "swo_debug.h"
#include "stdio-op.h"

#ifdef SWO_DEBUG_LOC
/*---------------------------------------------------------------------------*/
void 
SWO_init(void)
{
  uint32_t div;
  GPIO_Port_TypeDef swv_port = AF_DBG_SWV_PORT(SWO_DEBUG_LOC);
  uint8_t swv_pin = AF_DBG_SWV_PIN(SWO_DEBUG_LOC);
  /* Enable GPIO clock */
  CMU_ClockEnable(cmuClock_GPIO, true);
  /* Enable SWO output pin */
  GPIO_DbgSWOEnable(true);
  GPIO_DbgLocationSet(SWO_DEBUG_LOC);
  /* Enable Serial wire output pin */
  GPIO_PinModeSet(swv_port, swv_pin, gpioModePushPull, 0);
  /* Ensure auxiliary clock going to the Cortex debug trace module is enabled */
  CMU_OscillatorEnable(cmuOsc_AUXHFRCO, true, true);
  /* Enable trace in core debug. Must be enabled before using ITM */
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  div  = (CMU_ClockFreqGet(cmuClock_DBG) + (SWO_DEBUG_DEFAULT_CLOCK_HZ / 2)) / SWO_DEBUG_DEFAULT_CLOCK_HZ;
  TPIU->SPPR = 2;            /* set protocol to NRZ */
  TPIU->ACPR = div -1;       /* "Async Clock Prescaler Register". Scale the baud rate of the asynchronous output */
  ITM->LAR  = 0xC5ACCE55;   /* Unlock ITM and output data */
  ITM->TCR  = (ITM_TCR_TRACEBUSID_Msk | ITM_TCR_SWOENA_Msk | ITM_TCR_SYNCENA_Msk | ITM_TCR_ITMENA_Msk); //0x0001000D;
  ITM->TPR  = ITM_TPR_PRIVMASK_Msk; /* ITM Trace Privilege Register make channel 0 accessible by user code*/
  ITM->TER  = 0x1;          /* ITM Trace Enable Register. Enabled tracing on stimulus ports. One bit per stimulus port. */
  DWT->CTRL = 0x400003FE;
  TPIU->FFCR = 0x00000100;
}
/*---------------------------------------------------------------------------*/
void 
SWO_flush(void)
{
  while (((ITM->TCR & ITM_TCR_ITMENA_Msk) != 0UL)        /* ITM enabled */
         && ((ITM->TER & 1UL) != 0UL)                    /* ITM Port #0 enabled */
         && ((ITM->TCR & ITM_TCR_BUSY_Msk) != 0UL) ) {
    __NOP();
  }
}
/*---------------------------------------------------------------------------*/
#undef stdio_put_char_bw
void
stdio_put_char_bw(char c)
{
  ITM_SendChar(c);
}
#endif /* SWO_DEBUG_LOC */
