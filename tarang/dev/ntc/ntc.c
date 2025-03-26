/**
 * @file ntc.c
 * @author Varun Marolia
 * @brief Generic NTC thermistor driver. The driver supports NTC thermistor
 *        with beta value method and temperature chart method. The NTC cane
 *        be connected in two configurations, pulled up or pulled down.
 * @todo Add support for temperature chart method.
 * 
 * @copyright Copyright (c) 2025 Varun Marolia
 *   MIT License
 *   Permission is hereby granted, free of charge, to any person obtaining a copy
 *   of this software and associated documentation files (the "Software"), to deal
 *   in the Software without restriction, including without limitation the rights
 *   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *   copies of the Software, and to permit persons to whom the Software is
 *   furnished to do so, subject to the following conditions:**
 *
 *   The above copyright notice and this permission notice shall be included in all
 *   copies or substantial portions of the Software.
 *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *   SOFTWARE.
 * 
 */
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
  PRINTF("NTC: ADC mv:%lu\n", adc_mv);
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
  PRINTF("NTC: Resistance:%lu\n", (uint32_t)resistance);
  /* Convert the temperature to milli-Celsius */
  return (int32_t)(temperature * 1000);
}
#endif /* defined(__FPU_PRESENT) && (__FPU_PRESENT == 1) */
/*---------------------------------------------------------------------------*/
int32_t 
ntc_read_temp_mc_using_charts(ntc_thermistor_t *ntc)
{
  /**
   * @todo Implement method to calculate temperature value from resistance, temperature chart data.
   *       Chart can be generated using beta function or manually uploaded from manufacturer.
   *       The method must not be FPU dependent.
   * 
   */
  return 0;
}
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

