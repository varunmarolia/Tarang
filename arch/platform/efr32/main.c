#include "platform.h"
/*---------------------------------------------------------------------------*/
int
main(void) {
  platform_init();  /* this will intialize the board mcu peripherals */
  app_init();
  /* Poll all the functions periodically. */
  while(1) {
    app_poll();
  }
return 0;
}
