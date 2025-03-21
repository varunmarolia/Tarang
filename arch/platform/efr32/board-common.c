#include "board-common.h"
#include "clock.h"
#include "adc-dev.h"
#include <em_system.h>
#include <rail.h>
#include <stdio.h>

/*---------------------------------------------------------------------------*/
static void 
GPIO_common_IRQHandler()
{
  
}
/*---------------------------------------------------------------------------*/
void 
GPIO_EVEN_IRQHandler(void)
{
  GPIO_common_IRQHandler();
  NVIC_ClearPendingIRQ(GPIO_EVEN_IRQn);
}
/*---------------------------------------------------------------------------*/
void 
GPIO_ODD_IRQHandler(void)
{
  GPIO_common_IRQHandler();
  NVIC_ClearPendingIRQ(GPIO_ODD_IRQn);
}
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
button_reset_init(void) {
  GPIO_PinModeSet(RESET_PUSH_BUTTON_PORT, RESET_PUSH_BUTTON_PIN, 
                  gpioModeInput, 1);
  GPIO_ExtIntConfig(RESET_PUSH_BUTTON_PORT, RESET_PUSH_BUTTON_PIN, 
                    RESET_PUSH_BUTTON_PIN, false, true, false);
  const IRQn_Type irq = RESET_PUSH_BUTTON_PIN & 1 ? GPIO_ODD_IRQn : GPIO_EVEN_IRQn;
  GPIO_IntClear(1 << RESET_PUSH_BUTTON_PIN);
  NVIC_ClearPendingIRQ(irq);
  GPIO_IntEnable(1 << RESET_PUSH_BUTTON_PIN);
  NVIC_EnableIRQ(irq);
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
