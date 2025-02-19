#include "platform.h"
#include "tarang-version.h"
#include <stdio.h>
/*---------------------------------------------------------------------------*/
int
main(void) {
  platform_init();  /* this will intialize the board mcu peripherals */
  app_init();
  printf("Running Tarang " TARANG_VERSION_STRING " on " BOARD_NAME "\n");
  /* Poll all the functions periodically. */
  while(1) {
    app_poll();
  }
return 0;
}
