#include "platform.h"
#include "em_chip.h"
#include "clock.h"
#include "swo_debug.h"
#include "board-common.h"
#include "watchdog.h"

/*---------------------------------------------------------------------------*/
void
platform_init(void) 
{
  CHIP_Init();    /* Initialize the chip */
  board_init();   /* this will setup and select 38.4MHz Xtal and configure LEDs and button ports */
#ifdef USE_SWO_DEBUG
  SWO_init();
#endif  /* USE_SWO_DEBUG */
  clock_init();                       /* Initialize clock */
  button_reset_init();                /* initialize the reset button */
  watchdog_init(wdog_time_4s097);     /* Initialize watchdog timer */
  watchdog_guard();                   /* Enable watchdog timer */ 
}
/*---------------------------------------------------------------------------*/

