#include "board-common.h"
#include "clock.h"
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
