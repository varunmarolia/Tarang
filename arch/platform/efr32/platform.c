#include "platform.h"
#include "em_chip.h"
#include "clock.h"
#include "guart.h"
#include "swo_debug.h"
/*---------------------------------------------------------------------------*/
void
platform_init(void) 
{
  CHIP_Init();    /* Initialize the chip */
  board_init();   /* this will setup and select 38.4MHz Xtal and configure LEDs and button ports */
#ifdef SWO_DEBUG_LOC
  SWO_init();
#endif  /* SWO_DEBUG_LOC */
  clock_init();   /* Intialize clock */
  guart_init();   /* Intialize generic uart */
}
/*---------------------------------------------------------------------------*/

