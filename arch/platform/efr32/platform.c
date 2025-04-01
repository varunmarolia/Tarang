/**
 * @file platform.c
 * @author Varun Marolia
 * @brief This file contains platform specific calls. In this case efr32 SDK 
 *        specific calls. Each platform will have their own platform.c file 
 *        that will initialize the platform/board specific peripherals.
 *        The file also implements platform behavior specific functions such
 *        as common IO pin event handlers, managing platform sleep, reset 
 *        events etc. 
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
 
#include "platform.h"
#include "em_chip.h"
#include "clock.h"
#include "swo_debug.h"
#include "board-common.h"
#include "watchdog.h"

static void
reset_buton_handler(gpio_interrupt_t *button)
{
  /* Reset the system when reset button is pressed */
  (void)button;  /* avoid unused parameter warning */
  NVIC_SystemReset();
}
/*---------------------------------------------------------------------------*/
void
platform_init(void) 
{
  CHIP_Init();    /* Initialize the chip */
  board_init();   /* this will setup and select 38.4MHz Xtal and configure LEDs and button IO pins */
#ifdef USE_SWO_DEBUG
  SWO_init();
#endif  /* USE_SWO_DEBUG */
  clock_init();                       /* Initialize clock */
  RESET_BUTTON.callback = reset_buton_handler;  /* Set callback function for reset button */
  gpio_interrupt(&RESET_BUTTON, true);  /* Enable GPIO interrupt for reset button */
  watchdog_init(wdog_time_4s097);     /* Initialize watchdog timer */
  watchdog_guard();                   /* Enable watchdog timer */

}
/*---------------------------------------------------------------------------*/

