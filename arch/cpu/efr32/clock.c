/**
 * @file clock.c
 * @author Varun Marolia
 * @brief This file contains arch specific methods for common clock function.
 *        The clock is based on the system tick timer. The system tick timer
 *        is configured to generate interrupt at 1ms.
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

#include "clock.h"
#include "atomic-arch.h"
#include "em_device.h"

static volatile clock_time_t clock_ticks = 0;
static uint32_t usecond_clocks_10X;  /* 10 x number of system clocks required for 1 usecond */
/*---------------------------------------------------------------------------*/
#pragma GCC diagnostic ignored "-Wattributes" /* for GCC V12 it gives warning for FP regsiters minght be clobbered */
void SysTick_Handler(void) __attribute__ ((interrupt));
void
SysTick_Handler(void)
{
  ATOMIC_SECTION(clock_ticks++;);
}
/*---------------------------------------------------------------------------*/
void 
clock_init(void)
{
  uint32_t sys_clk = SystemCoreClockGet();  /* system clock freq in HZ */
  /* board specfic HFXO crystal should be selected and enabled in the board_init function under board.c files */
  /* set system tick to generate interrupt at 1ms. Accuracy depends upon crystal tune */
  SysTick_Config(sys_clk / CLOCK_TICKS_CONF);
  usecond_clocks_10X = sys_clk / 100000;  /* for 38.4 MHz clock. This would be 384 */
}
/*---------------------------------------------------------------------------*/
clock_time_t
clock_get_ticks(void)
{
  return clock_ticks;
}
/*---------------------------------------------------------------------------*/
clock_time_t
clock_get_time_ms(void)
{
  return ((clock_ticks * 1000) / CLOCK_TICKS_CONF);
}
/*---------------------------------------------------------------------------*/
clock_time_t
clock_get_seconds(void)
{
  return (clock_get_time_ms() / 1000);
}
/*---------------------------------------------------------------------------*/
void
clock_wait_ms(clock_time_t time_ms)
{
  clock_time_t start;
  start = clock_get_time_ms();
  while(clock_get_time_ms() - start < time_ms); 
}
/*---------------------------------------------------------------------------*/
void
clock_delay_us(uint32_t time_us)
{
  /* calcualte number of loops required for passing 1 us 
   * It takes roughly 5 cycles on ARM processor to down count to zero
   * remove approximate 20 cycles for calcualtions
   */
  uint32_t loops;
  if(time_us) {
    loops = (((usecond_clocks_10X * time_us) / 10) - 20) / 5;
    while(loops > 0) {
      loops--;
    }
  }
}
/*---------------------------------------------------------------------------*/
