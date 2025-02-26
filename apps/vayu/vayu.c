#include "board.h"
#include "sht4x.h"
#include "serial-status.h"
#include <stdio.h>
#include "guart.h"

uint32_t temperature_mk;
int8_t temperature_c;
int8_t temperature_c_fraction;
uint32_t humidity_ppm;
uint8_t humidity_rh;
uint8_t humidity_rh_fraction;
/*---------------------------------------------------------------------------*/
uint8_t 
app_init(void) {
  guart_init();   /* Initialize generic UART */
  sht4x_init();
  return 0;
}
/*---------------------------------------------------------------------------*/
void
app_poll(void) {
  uint8_t bus_status;
  bus_status = sht4x_take_single_measurement(&temperature_mk, &humidity_ppm);
  if(bus_status == BUS_OK) {
    humidity_rh = humidity_ppm / 10000; /* convert PPM into %RH */
    humidity_rh_fraction = (humidity_ppm % 10000) / 100;

    temperature_c = (temperature_mk - 273150) / 1000;
    temperature_c_fraction = ((temperature_mk - 273150) % 1000 / 10);
    printf("App_poll: temperature:%02d.%02u'C humidity:%02u.%02uRH\n", temperature_c, temperature_c_fraction, humidity_rh, humidity_rh_fraction);
  } else {
    printf("App_poll: Failed to read measurement!!!\n");
  }
}
/*---------------------------------------------------------------------------*/

