#include "platform.h"
#include "clock.h"
#include "guart.h"
/*---------------------------------------------------------------------------*/
void
platform_init(void) {
  board_init();   /* this will setup Xtals */
  clock_init();   /* Intialize clock */
  guart_init();   /* Intialize generic uart */
}
/*---------------------------------------------------------------------------*/

