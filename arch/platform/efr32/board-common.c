/**
 * @file board-common.c
 * @author Varun Marolia
 * @brief   This file contains functions to initialize and use common board peripherals like LEDs, buttons, etc.
 *          each board should have a Reset switch connected to a GPIO pin that could wake up the MCU from deep sleep.
 *          And 2 LEDs for system status indication. A power switch should be installed for battery operated devices.
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

#include "board-common.h"
#include "clock.h"
#include <em_system.h>
#include <rail.h>
#include <stdio.h>

/*---------------------------------------------------------------------------*/
void
led_sys_set(gpio_port_t port, uint8_t pin, uint8_t on_off)
{
  if(on_off) {
    gpio_set_pin_logic(port, pin, GPIO_PIN_LOGIC_LOW);
  } else {
    gpio_set_pin_logic(port, pin, GPIO_PIN_LOGIC_HIGH);
  }
}
/*---------------------------------------------------------------------------*/
void 
led_sys_blink(gpio_port_t port, uint8_t pin, uint8_t times, uint32_t delay_ms)
{
  while(times) {
    led_sys_set(port, pin, LED_SYS_ON);
    clock_wait_ms(delay_ms);
    led_sys_set(port, pin, LED_SYS_OFF);
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
uint32_t
board_read_voltage_divider_mv(adc_dev_t *dev, uint32_t r1, uint32_t r2)
{
  uint32_t adc_uv = 0;
  uint64_t mv = 0;
  if(dev && r2) {
    adc_dev_read_microvolts(dev, &adc_uv);
    mv = adc_uv / 1000; /* convert into millivolts */
    mv *= (r1 + r2);
    mv /= r2;         /* voltage divider ratio */
  }
  return (uint32_t)mv;
}
/*---------------------------------------------------------------------------*/

