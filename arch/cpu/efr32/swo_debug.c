/**
 * @file swo_debug.c
 * @author Varun Marolia
 * @brief This file contains arch specific methods for using Serial wire output 
 *        (SWO) for debugging. In order to use this the USE_SWO_DEBUG must be
 *        defined in the project configuration/Make file.
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
#include "em_device.h"
#include "em_gpio.h"
#include "em_cmu.h"
#include "board.h"
#include "swo_debug.h"
#include "stdio-op.h"

/*!
 \todo SWO output is not working!
 */
#ifdef USE_SWO_DEBUG
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
  /* The default AUXHFRCO frequency is 19 MHz. */
  CMU_OscillatorEnable(cmuOsc_AUXHFRCO, true, true);
  /* Set AUXHFRCO frequency to 4 MHz */
  CMU_AUXHFRCOBandSet(cmuAUXHFRCOFreq_4M0Hz);
  /* Enable trace in core debug. Must be enabled before using ITM */
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  //div  = (CMU_ClockFreqGet(cmuClock_DBG) + (SWO_DEBUG_DEFAULT_CLOCK_HZ / 2)) / SWO_DEBUG_DEFAULT_CLOCK_HZ;
  div = (4000000 / SWO_DEBUG_DEFAULT_CLOCK_HZ) - 1;
  TPIU->SPPR = 2;            /* set protocol to NRZ */
  TPIU->ACPR = div -1;       /* Scale the baud rate of the asynchronous output */
  ITM->LAR  = 0xC5ACCE55;    /* Unlock ITM and output data */
  ITM->TCR  = (ITM_TCR_TRACEBUSID_Msk | ITM_TCR_DWTENA_Msk| ITM_TCR_SWOENA_Msk 
              | ITM_TCR_SYNCENA_Msk | ITM_TCR_ITMENA_Msk); /* ITM Trace Control Register */
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
#endif /* USE_SWO_DEBUG */
