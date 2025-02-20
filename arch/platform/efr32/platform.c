#include "platform.h"
#include "em_chip.h"
#include "clock.h"
#include "guart.h"
/*---------------------------------------------------------------------------*/
void
platform_init(void) 
{
  CHIP_Init();    /* Initialize the chip */
  board_init();   /* this will setup and select 38.4MHz Xtal and configure LEDs and button ports */
  clock_init();   /* Intialize clock */
  guart_init();   /* Intialize generic uart */
}
/*---------------------------------------------------------------------------*/

