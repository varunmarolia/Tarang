#include "platform.h"
#include "tarang-version.h"
#include "timer.h"
#include "guart.h"
#include "board-common.h"
#include <stdio.h>
/*---------------------------------------------------------------------------*/
int
main(void) {
  ttimer_t poll_timer;
  platform_init();  /* this will intialize the board mcu peripherals */
  app_init();
  led_blink(LED_SYS_GREEN_PORT, LED_SYS_GREEN_PIN, 2, 250);
  printf("Running Tarang " TARANG_VERSION_STRING " on " BOARD_NAME "\n");
  printf("Arch info:\n");
  print_chip_info();
  timer_set(&poll_timer,  0);
  /* Poll all the functions periodically. */
  while(1) {
    while(!timer_timedout(&poll_timer));
    app_poll();
    timer_set(&poll_timer, 3000);
    led_blink(LED_SYS_YELLOW_PORT, LED_SYS_YELLOW_PIN, 1, 100);
  }
return 0;
}
