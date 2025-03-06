#include "board.h"
#include <stdio.h>
#include "sht4x.h"
#include "guart.h"
#include "ntc.h"

#define NTC_BOARD 0
#define NTC_HRV   1
#define NTC_TOTAL 2
extern adc_dev_t HA_NTC_ADC_DEV;
extern adc_dev_t BOARD_NTC_ADC_DEV;
extern serial_dev_t SHT4X_DEV;
/*---------------------------------------------------------------------------*/
sht4x_t sht4x_sensor = {
  .last_rh_ppm = 0,
  .last_temp_mk = 0,
  .serial_number = 0,
  .sht4x_dev = &SHT4X_DEV
};
/*---------------------------------------------------------------------------*/
ntc_thermistor_t ntc_dev[NTC_TOTAL] = {
  { /* board NTC details */
    .adc_dev = &BOARD_NTC_ADC_DEV,
    .beta_value_25 = 4250,
    .known_resistance_ohm = 200000,
    .max_negative_temp_C = -20,
    .max_positive_temp_C = 125,
    .ntc_config = NTC_PULLED_DOWN_CONFIG,
    .R0_ohm = 100000,
    .RT_chart = NULL,
    .supply_mV = 3300,
    .T0_C = 25,
    .temp_step = 0
  },
  { /* HRV NTC details */
    .adc_dev = &HA_NTC_ADC_DEV,
    .beta_value_25 = 3950,
    .known_resistance_ohm = 100000,
    .max_negative_temp_C = -20,
    .max_positive_temp_C = 125,
    .ntc_config = NTC_PULLED_DOWN_CONFIG,
    .R0_ohm = 100000,
    .RT_chart = NULL,
    .supply_mV = 3300,
    .T0_C = 25,
    .temp_step = 0
  }
};
/*---------------------------------------------------------------------------*/
static void
read_sht4x(void)
{
  int8_t temperature_c;
  int8_t temperature_c_fraction;
  uint8_t humidity_rh;
  uint8_t humidity_rh_fraction;
  uint8_t bus_status;
  bus_status = sht4x_take_single_measurement(&sht4x_sensor);
  if(bus_status == BUS_OK) {
    humidity_rh = sht4x_sensor.last_rh_ppm / 10000; /* convert PPM into %RH */
    humidity_rh_fraction = (sht4x_sensor.last_rh_ppm % 10000) / 100;

    temperature_c = (sht4x_sensor.last_temp_mk - 273150) / 1000;
    temperature_c_fraction = ((sht4x_sensor.last_temp_mk - 273150) % 1000 / 10);
    printf("App_poll: sht4x temperature:%02d.%02u'C humidity:%02u.%02uRH\n", temperature_c, temperature_c_fraction, humidity_rh, humidity_rh_fraction);
  } else {
    printf("App_poll: Failed to read measurement!!!\n");
  }
}
/*---------------------------------------------------------------------------*/
static void
read_ntc(uint8_t ntc_type) 
{
  uint32_t temp_mC = 0;
  if(ntc_type >= NTC_TOTAL) { 
    return;
  }
  temp_mC = ntc_read_temp_mc_using_beta(&ntc_dev[ntc_type]);
  if(temp_mC == NTC_ERROR) {
    printf("App_poll: NTC error\n");
  } else {
    switch(ntc_type) {
      case NTC_HRV:
        printf("App_poll: NTC_HRV ");
      break;
      case NTC_BOARD:
        printf("App_poll: NTC_BOARD ");
      break;
      default:
      break;
    }
    printf("temperature:%03u.%02u 'C\n", (uint16_t)(temp_mC / 1000), (uint16_t)(temp_mC % 1000) / 10);
  }
}
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
uint8_t 
app_init(void) {
  guart_init();   /* Initialize generic UART */
  sht4x_init(&sht4x_sensor);
  return 0;
}
/*---------------------------------------------------------------------------*/
void
app_poll(void) {
  read_sht4x();
  read_ntc(NTC_HRV);
  read_ntc(NTC_BOARD);
}
/*---------------------------------------------------------------------------*/

