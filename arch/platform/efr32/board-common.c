#include "board-common.h"
#include "clock.h"
#include "adc-dev.h"
#include <em_system.h>
#include <rail.h>
#include <stdio.h>
/*---------------------------------------------------------------------------*/
void
led_set(GPIO_Port_TypeDef port, uint32_t pin, uint8_t on_off)
{
  if(on_off == LED_ON) {
    GPIO_PinOutClear(port, pin);
  } else {
    GPIO_PinOutSet(port, pin);
  }
}
/*---------------------------------------------------------------------------*/
void 
led_blink(GPIO_Port_TypeDef port, uint32_t pin, uint8_t times, uint32_t delay_ms)
{
  while(times) {
    led_set(port, pin, LED_ON);
    clock_wait_ms(delay_ms);
    led_set(port, pin, LED_OFF);
    clock_wait_ms(delay_ms);
    times--;
  }
}
/*---------------------------------------------------------------------------*/
void
print_chip_info(void)
{
  RAIL_Version_t rail_ver;
  SYSTEM_ChipRevision_TypeDef rev;
  uint16_t part_number;
  uint8_t prod_rev;
  uint8_t dev_rev;

  SYSTEM_ChipRevisionGet(&rev);
  prod_rev = SYSTEM_GetProdRev();
  dev_rev = SYSTEM_GetDevinfoRev();
  part_number = SYSTEM_GetPartNumber();
  printf("EFR32 %u.%u (0x%x)  Prod rev: %u  Dev rev: %u  Part no: %u\n",
         rev.major, rev.minor, rev.family, prod_rev,
         dev_rev, part_number);
  RAIL_GetVersion(&rail_ver, false);
  printf("RAIL  %u.%u.%u.%u [%s]\n",
         rail_ver.major, rail_ver.minor, rail_ver.rev, rail_ver.build,
         rail_ver.multiprotocol ? "multi" : "single");
}
/*---------------------------------------------------------------------------*/
#ifdef BOARD_SUPPLY_TEMP_ADC_INPUT
static float
power(float x, uint32_t exp)
{
  uint32_t i;
  float ret = 1.0f;
  for(i = 0; i < exp; i++) {
    ret *= x;
  }
  return ret;
}
/*---------------------------------------------------------------------------*/
extern adc_dev_t BOARD_NTC_ADC_DEV;
int32_t
board_sensors_get_temp_mcelsius(void)
{
  /* Polynomial coefficients, in ascending order */
  static const float polynomial_coeffs[] = {
    8.5273e+04,
    -9.1439e+01,
    5.0424e-02,
    -1.5738e-05,
    1.7684e-09,
  }; /* The polynomial is defined for 3600 mVolt input voltage */
  const uint32_t polynomial_tabel_length =  (sizeof(polynomial_coeffs) / sizeof(float));
  uint32_t milli_volt;
  float temp_mc = 0;
  uint32_t i;

  adc_dev_read_single(&BOARD_NTC_ADC_DEV, &milli_volt);
  milli_volt *= 3600; /* because the polynomial is defined for 3600 mVolt input voltage */
  milli_volt /= ADC_RESOLUTION; /* conversion of ADC value to millivolt */
  
  for(i = 0; i < polynomial_tabel_length; i++) {
    temp_mc += polynomial_coeffs[i] * power(milli_volt, i);
    /* [milliCelsius] */
  }
  return (int32_t)temp_mc;
}
/*---------------------------------------------------------------------------*/
extern adc_dev_t BOARD_SUPPLY_ADC_DEV;
uint32_t
board_sensors_get_mvoltage(void)
{
  uint32_t mv = 0;
  adc_dev_read_microvolts(&BOARD_SUPPLY_ADC_DEV, &mv);
  mv /= 1000; /* convert into millivolts */
  mv *= 3;    /* voltage divider ratio is 3 */
  return mv;
}
#endif  /* BOARD_SUPPLY_TEMP_ADC_INPUT */
