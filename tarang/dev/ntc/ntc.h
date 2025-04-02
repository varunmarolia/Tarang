#ifndef _NTC_H_
#define _NTC_H_

#include "adc-dev.h"

typedef enum ntc_config {
  NTC_PULLED_UP_CONFIG = 0,
  NTC_PULLED_DOWN_CONFIG = 1
} ntc_config_t;

#define NTC_ERROR -300000          /* could note read NTC value. Range outside the possible temperature range */

typedef struct ntc_thermistor {
  const uint16_t beta_value_25;                 /* Beta value at 25 */
  const uint32_t R0_ohm;                        /* NTC resistance value in ohms at T0 room temperature */
  const int16_t T0_C;                           /* room temperature value in Celsius */
  const uint32_t known_resistance_ohm;          /* know resistance value in the voltage divider */
  const ntc_config_t ntc_config;                /* configuration of NTC, NTC_PULLED_UP_CONFIG OR NTC_PULLED_DOWN_CONFIG */
  const int16_t max_negative_temp_C;            /* Max negative measurable temperature value for this NTC */
  const uint16_t max_positive_temp_C;           /* Max positive measurable temperature value for this NTC */
  const uint16_t supply_mV;                     /* VCC of the voltage divider */
  /* arch specific */
  adc_dev_t *adc_dev;                           /* pointer to the ADC value */
  const uint16_t *RT_chart;                     /* pointer to the Resistance temperature chart array */
  const uint8_t temp_step;                      /* single step temperature increment in the RT table */
} ntc_thermistor_t;

#if defined(__FPU_PRESENT) && (__FPU_PRESENT == 1)
int32_t ntc_read_temp_mc_using_beta(ntc_thermistor_t *ntc);
#endif  /* defined(__FPU_PRESENT) && (__FPU_PRESENT == 1) */
int32_t ntc_read_temp_mc_using_charts(ntc_thermistor_t *ntc);

#endif  /* _NTC_H_ */
