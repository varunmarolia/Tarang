/**
 * @file gpio-arch.c
 * @author Varun Marolia
 * @brief This file implements arch specific methods for GPIO driver functions
 *        defined in gpio.h.
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

#include "gpio.h"
#include <stddef.h>
#include <em_gpio.h>
#include <em_cmu.h>
#include "clock.h"

#define MAX_EXT_INT 16  /* maximum number of external interrupts */

struct gpio_interrupt *interrupts[MAX_EXT_INT] = {NULL};
/*---------------------------------------------------------------------------*/
static void 
GPIO_common_IRQHandler()
{
  uint32_t int_flags;
  clock_time_t time_stamp_ms;
  uint8_t i;
  time_stamp_ms = clock_get_time_ms() & 0xFFFFFFFF; /* get current time last 32 bits only */
  int_flags = GPIO_IntGetEnabled();
  for(i = 0; i < MAX_EXT_INT; i++) {
    if(int_flags & (1 << i)) {
      if(interrupts[i] != NULL) {
        switch(interrupts[i]->int_mode) {
          case GPIO_MODE_INPUT_EXTERNAL_PULL_UP:
          case GPIO_MODE_INPUT_INTERNAL_PULL_UP:
            if(gpio_get_pin_logic(interrupts[i]->port, interrupts[i]->pin) == GPIO_PIN_LOGIC_LOW) {
              interrupts[i]->pulse_start_ts_ms = (uint32_t)time_stamp_ms; /* set pulse start time */
            } else {
              interrupts[i]->pulse_time_ms = time_stamp_ms - interrupts[i]->pulse_start_ts_ms; /* calculate the pulse time */
            }
          break;

          case GPIO_MODE_INPUT_EXTERNAL_PULL_DOWN:
          case GPIO_MODE_INPUT_INTERNAL_PULL_DOWN:
            if(gpio_get_pin_logic(interrupts[i]->port, interrupts[i]->pin) == GPIO_PIN_LOGIC_HIGH) {
              interrupts[i]->pulse_start_ts_ms = (uint32_t)time_stamp_ms; /* set pulse start time */
            } else {
              interrupts[i]->pulse_time_ms = time_stamp_ms - interrupts[i]->pulse_start_ts_ms; /* calculate the pulse time */
            }
          break;

          default:
          break;
        }
        /* call the callback function */
        if(interrupts[i]->callback != NULL) {
          interrupts[i]->callback(interrupts[i]);
        }
      }
      GPIO_IntClear(1 << i);
      break;
    }
  }
}
/*---------------------------------------------------------------------------*/
#pragma GCC diagnostic ignored "-Wattributes" /* for GCC V12 it gives warning of FP registers might be clobbered */
void GPIO_EVEN_IRQHandler(void) __attribute__((interrupt));
void 
GPIO_EVEN_IRQHandler(void)
{
  GPIO_common_IRQHandler();
  NVIC_ClearPendingIRQ(GPIO_EVEN_IRQn);
}
/*---------------------------------------------------------------------------*/
void GPIO_ODD_IRQHandler(void) __attribute__((interrupt));
void 
GPIO_ODD_IRQHandler(void)
{
  GPIO_common_IRQHandler();
  NVIC_ClearPendingIRQ(GPIO_ODD_IRQn);
}
/*---------------------------------------------------------------------------*/
static 
GPIO_Mode_TypeDef get_efr32_gpio_mode(gpio_mode_t mode)
{
  switch (mode) {
    case GPIO_MODE_DISABLED:
      return gpioModeDisabled;
    case GPIO_MODE_DISABLED_INTERNAL_PULL_UP:
      return gpioModeDisabled;
    case GPIO_MODE_INPUT:
      return gpioModeInput;
    case GPIO_MODE_INPUT_EXTERNAL_PULL_UP:
      return gpioModeInput;
    case GPIO_MODE_INPUT_EXTERNAL_PULL_DOWN:
      return gpioModeInput;
    case GPIO_MODE_INPUT_INTERNAL_PULL_UP:
      return gpioModeInputPull;
    case GPIO_MODE_INPUT_INTERNAL_PULL_DOWN:
      return gpioModeInputPull;
    case GPIO_MODE_OUTPUT_PUSH_PULL_SET:
      return gpioModePushPull;
    case GPIO_MODE_OUTPUT_PUSH_PULL_CLEAR:
      return gpioModePushPull;
    case GPIO_MODE_OUTPUT_OPEN_SOURCE:
      return gpioModeWiredOr;
    case GPIO_MODE_OUTPUT_OPEN_SOURCE_INTERNAL_PULL_DOWN:
      return gpioModeWiredOrPullDown;
    case GPIO_MODE_OUTPUT_OPEN_DRAIN:
      return gpioModeWiredAnd;
    case GPIO_MODE_OUTPUT_OPEN_DRAIN_INTERNAL_PULL_UP:
      return gpioModeWiredAndPullUp;
    default:
      break;
  }
  return gpioModeDisabled; /* default to disabled */
}
/*---------------------------------------------------------------------------*/
void 
gpio_init(void)
{
  CMU_ClockEnable(cmuClock_GPIO, true);
}
/*---------------------------------------------------------------------------*/
void 
gpio_set_mode(gpio_port_t port, uint8_t pin, gpio_mode_t mode, uint8_t out)
{
  GPIO_PinModeSet((GPIO_Port_TypeDef)port, pin, get_efr32_gpio_mode(mode), out);
}
/*---------------------------------------------------------------------------*/
uint8_t 
gpio_get_pin_logic(gpio_port_t port, uint8_t pin)
{
  return (uint8_t)GPIO_PinOutGet((GPIO_Port_TypeDef)port, pin);
}
/*---------------------------------------------------------------------------*/
void gpio_set_pin_logic(gpio_port_t port, uint8_t pin, gpio_pin_logic_t logic)
{
  if (logic == GPIO_PIN_LOGIC_LOW) {
    GPIO_PinOutClear((GPIO_Port_TypeDef)port, pin);
  } else {
    GPIO_PinOutSet((GPIO_Port_TypeDef)port, pin);
  }
}
/*---------------------------------------------------------------------------*/
void 
gpio_toggle_pin_logic(gpio_port_t port, uint8_t pin)
{
  GPIO_PortOutToggle((GPIO_Port_TypeDef)port, (1 << pin));
}
/*---------------------------------------------------------------------------*/
void 
gpio_interrupt(gpio_interrupt_t *gpio_interrupt, bool enable)
{
  if(gpio_interrupt != NULL) {
    /* disable the interrupt before configuring the IO pin */
    GPIO_IntDisable(1 << gpio_interrupt->int_no);
    /* set up gpio mode */
    GPIO_PinModeSet((GPIO_Port_TypeDef)gpio_interrupt->port, gpio_interrupt->pin,
                    get_efr32_gpio_mode(gpio_interrupt->mode), 
                    (gpio_interrupt->mode == GPIO_MODE_INPUT_INTERNAL_PULL_UP) ? 1 : 0);
    /* configure interrupt */
    GPIO_ExtIntConfig((GPIO_Port_TypeDef)gpio_interrupt->port, gpio_interrupt->pin, gpio_interrupt->int_no,
                      (gpio_interrupt->int_mode == GPIO_INTERRUPT_MODE_RISING_EDGE) ? true : false,
                      (gpio_interrupt->int_mode == GPIO_INTERRUPT_MODE_FALLING_EDGE) ? true : false,
                      enable);
    /* configure EM4U deep sleep interrupt */
    if(gpio_interrupt->low_power_interrupt) {
      GPIO_EM4WUExtIntConfig((GPIO_Port_TypeDef)gpio_interrupt->port, gpio_interrupt->pin, gpio_interrupt->int_no,
                          ((gpio_interrupt->int_mode == GPIO_INTERRUPT_MODE_RISING_EDGE) ||
                          (gpio_interrupt->int_mode == GPIO_INTERRUPT_MODE_LEVEL_HIGH)) ? true : false, enable);
    }
    /* assign the interrupt device to global interrupt device array */
    interrupts[gpio_interrupt->int_no] = gpio_interrupt;
    /* enable the corresponding interrupt */
    if(enable) {
      const IRQn_Type irq = gpio_interrupt->pin & 1 ? GPIO_ODD_IRQn : GPIO_EVEN_IRQn;
      GPIO_IntClear(1 << gpio_interrupt->pin);
      NVIC_ClearPendingIRQ(irq);
      GPIO_IntEnable(1 << gpio_interrupt->pin);
      NVIC_EnableIRQ(irq);
    }
  }
}
/*---------------------------------------------------------------------------*/
void 
gpio_clear_interrupt(gpio_interrupt_t *gpio_interrupt)
{
  if(gpio_interrupt != NULL) {
    GPIO_IntClear(1 << gpio_interrupt->pin);
  }
}
/*---------------------------------------------------------------------------*/
void 
gpio_set_interrupt_callback(gpio_interrupt_t *gpio_interrupt, 
                            void (*callback)(gpio_interrupt_t *ptr))
{
  if(gpio_interrupt != NULL) {
    interrupts[gpio_interrupt->int_no] = gpio_interrupt;
    interrupts[gpio_interrupt->int_no]->callback = callback;
  }
}
/*---------------------------------------------------------------------------*/
