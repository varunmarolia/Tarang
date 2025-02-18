#include "board.h"
#include "sht4x.h"

uint32_t sht4x_temp_mk = 0;
uint32_t sht4x_rh = 0;
/*---------------------------------------------------------------------------*/
uint8_t 
app_init(void) {
  sht4x_init();
  return 0;
}
/*---------------------------------------------------------------------------*/
void
app_poll(void) {
  sht4x_take_single_measurement(&sht4x_temp_mk, &sht4x_rh);
}
/*---------------------------------------------------------------------------*/

