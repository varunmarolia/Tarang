#include "ntc.h"
#include "board.h"
#include "adc-dev.h"
#include <math.h>
#define NTC_DEBUG 0
#if NTC_DEBUG
#include <stdio.h>
#undef PRINTF
#define PRINTF(...) printf(__VA_ARGS__)
#else  /* NTC_DEBUG */
/**< Replace printf with nothing */
#define PRINTF(...)
#endif /* NTC_DEBUG */
/*---------------------------------------------------------------------------*/
#if defined(__FPU_PRESENT) && (__FPU_PRESENT == 1)
int32_t 
ntc_read_temp_mc_using_beta(ntc_thermistor_t *ntc)
{
  uint32_t adc_mv;
  double resistance;
  double temperature;
  
  if (adc_dev_read_microvolts(ntc->adc_dev, &adc_mv) != ADC_OK) {
      return 0;
  }
  /* convert reading into millivolts */
  adc_mv /= 1000;
  printf("NTC: ADC mv:%lu\n", adc_mv);
  /* Calculate the resistance of the NTC thermistor using voltage divider rule */
  if (ntc->ntc_config == NTC_PULLED_DOWN_CONFIG) {
      resistance = ntc->known_resistance_ohm * ((double)adc_mv / (ntc->supply_mV - adc_mv));
  } else { // NTC_PULL_UP
      resistance = ntc->known_resistance_ohm * ((double)(ntc->supply_mV - adc_mv) / adc_mv);
  }
  /* Calculate the temperature in Kelvin using the beta parameter equation */
  temperature = 1.0 / ((1.0 / (ntc->T0_C + 273.15)) + (1.0 / ntc->beta_value_25) * log(resistance / ntc->R0_ohm));
  /* Convert the temperature to Celsius */
  temperature -= 273.15;
  printf("NTC: Resistance:%lf\n", resistance);
  /* Convert the temperature to milli-Celsius */
  return (int32_t)(temperature * 1000);
}
#endif /* defined(__FPU_PRESENT) && (__FPU_PRESENT == 1) */
/*---------------------------------------------------------------------------*/
int32_t 
ntc_read_temp_mc_using_charts(ntc_thermistor_t *ntc)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

