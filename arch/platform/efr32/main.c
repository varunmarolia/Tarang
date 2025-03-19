#include "platform.h"
#include "tarang-version.h"
#include "timer.h"
#include "guart.h"
#include "board-common.h"
#include <stdio.h>
#include "watchdog.h"
/*---------------------------------------------------------------------------*/
int
main(void) {
  ttimer_t poll_timer;
  uint8_t guart_rx[GUART_RX_BUFFER_SIZE + 1];
  uint8_t guart_read_bytes;
  extern guart_t uart_debug;
  platform_init();  /* this will initialize the board MCU peripherals */
  app_init();
  led_blink(LED_SYS_GREEN_PORT, LED_SYS_GREEN_PIN, 2, 250);
  printf("Running Tarang " TARANG_VERSION_STRING " on " BOARD_NAME "\n");
  printf("Arch info:\n");
  print_chip_info();
  timer_set(&poll_timer,  0);
  /* Poll all the functions periodically. */
  while(1) {
    guart_read_bytes = guart_read_line(&uart_debug, guart_rx);
    if(guart_read_bytes) {
      guart_rx[guart_read_bytes] = '\0';
      printf("Main: received: %s\n", guart_rx);
    }
    if(timer_timedout(&poll_timer)) {
      app_poll();
      timer_set(&poll_timer, 3000);
      led_blink(LED_SYS_YELLOW_PORT, LED_SYS_YELLOW_PIN, 1, 100);
    }
    watchdog_feed();
  }
return 0;
}
