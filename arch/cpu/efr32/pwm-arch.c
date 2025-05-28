/**
 * @file pwm-arch.c
 * @author Varun Marolia
 * @brief This file contains arch specific methods for using Pulse Width 
 *        Modulation feature of the EFR32 MCU. 
 *        - A PWM timer can be used by multiple devices with different 
 *          compare channels at the same time.
 *        - Current implementation works with TIMER0 and TIMER1 only.
 *        - @todo Add support for WTIMER0 and LETIMER0.
 * 
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
#include "pwm-arch.h"
#include "pwm-dev.h"
#include <em_cmu.h>

#define DEBUG_PWM_ARCH 0     /**< Set this to 1 for debug printf output */
#if DEBUG_PWM_ARCH
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)      /**< Replace printf with nothing */
#endif /* PWM_ARCH */

#ifdef TIMER0
const pwm_config_t *pwm_config_timer0 = NULL;
#endif /* TIMER0 */

#ifdef TIMER1
const pwm_config_t *pwm_config_timer1 = NULL;
#endif /* TIMER1 */
/*---------------------------------------------------------------------------*/
pwm_status_t 
pwm_arch_init(pwm_dev_t *dev)
{
  GPIO_Port_TypeDef pwm_port;
  uint8_t pwm_pin;
  uint32_t counter_top = CMU_ClockFreqGet(cmuClock_HFPER) / dev->config->freq_hz - 1;
  uint32_t compare_value = (counter_top * dev->duty_cycle_100x) / 10000;
  if(!TIMER_REF_VALID(dev->config->timer_per)) {
    PRINTF("PWM-ARCH: Given timer is not supported by this arch !!!\n");
    return PWM_STATUS_INVALID_TIMER;
  }
  /* Select CC (capture and compare) channel parameters. */
  TIMER_InitCC_TypeDef timerCCInit = {
    .eventCtrl  = timerEventEveryEdge,      /* Input capture event control. Ignored in PWM mode */
    .edge       = timerEdgeBoth,            /* Input capture on both edges. Ignored in PWM mode */
    .prsSel     = timerPRSSELCh0,           /* select PRS channel 0. Not using PRS  */
    .cufoa      = timerOutputActionNone,    /* counter underflow output action-> No action */
    .cofoa      = timerOutputActionNone,    /* counter overflow output action-> No action */
    .cmoa       = timerOutputActionToggle,  /* counter match output action -> toggle output on match */
    .mode       = timerCCModePWM,           /* compare/capture channel mode -> PWM mode */
    .filter     = false,                    /* disable (false) digital filter */
    .prsInput   = false,                    /* Select TIMERnCCx (false) or PRS as input (true). */
    .coist      = true,                     /* Compare output initial state when the counter is disabled. High (true) Low (false) */
    .outInvert  = false,                    /* In up count mode output is cleared on compare match */
  };
  if(dev->pwm_active_logic == ENABLE_ACTIVE_LOW) {
    timerCCInit.outInvert = true;
  }
  /* Select timer parameters */
  TIMER_Init_TypeDef timerInit = {
    .enable     = true,                 /* Enable timer */
    .debugRun   = true,                 /* Stop counter during debug halt */
    .prescale   = timerPrescale1,       /* prescaler HFPER_clk/1 */
    .clkSel     = timerClkSelHFPerClk,  /* Select HFPER as clock source */
    .fallAction = timerInputActionNone, /* No action on falling input edge while in capture mode */
    .riseAction = timerInputActionNone, /* No action on rising input edge while in capture mode */
    .mode       = timerModeUp,          /* timer mode in Up counting. in UP mode timer is cleared on compare match - set on TOP */
    .dmaClrAct  = false,                /* Do not clear DMA request when DMA channel is active */
    .quadModeX4 = false,                /* Select X2 quadrature decode mode (if used) */
    .oneShot    = false,                /* Disable one shot */
    .sync       = false,                /* Not started/stopped/reloaded by other timers */
  };
  CMU_ClockEnable(cmuClock_GPIO, true);
#ifdef TIMER0
  if(dev->config->timer_per == TIMER0) {
    /* configure the timer only if the config is different than what is already set */
    if(dev->config != pwm_config_timer0) {
      if(pwm_config_timer0 != NULL) {
        /* timer is set in different config already by some other device! */
        PRINTF("PWM-ARCH: Timer0 is already in use by some other device!!!\n");
        return PWM_STATUS_BUSY;
      } 
      /* enable the timer clock frequency */
      CMU_ClockEnable(cmuClock_TIMER0, true);
      /* Set Top Value, TOP=(F_HFPER/(2^PRESC x F_PWM))-1 here PRESC value = 0 */
      TIMER_TopSet(TIMER0, counter_top);
      PRINTF("PWM-ARCH: Set Timer0 Top: %lu\n", counter_top);
    } else {
      PRINTF("PWM-ARCH: Timer0 is already configured with same config.\n");
    }
    /* Configure CC channel */
    TIMER_InitCC(TIMER0, dev->cc_channel, &timerCCInit);
    /* Set compare value for PWM compare channel (CC) to compare value. */
    TIMER_CompareBufSet(TIMER0, dev->cc_channel, compare_value);
    /* set up port, pin out for selected channel */
    switch (dev->cc_channel) {
      case 0:
        pwm_port = AF_TIMER0_CC0_PORT(dev->gpio_loc);
        pwm_pin = AF_TIMER0_CC0_PIN(dev->gpio_loc);
        /* Route the location of PWM channel to output pin */
        TIMER0->ROUTELOC0 = (TIMER0->ROUTELOC0 & ~_TIMER_ROUTELOC0_CC0LOC_MASK)
        | (dev->gpio_loc << _TIMER_ROUTELOC0_CC0LOC_SHIFT);
        TIMER0->ROUTEPEN |= TIMER_ROUTEPEN_CC0PEN; /*CC Channel 0 pin Enable */
        PRINTF("PWM-ARCH: Timer0 CC channel 0 is configured.\n");
      break;
      
      case 1:
        pwm_port = AF_TIMER0_CC1_PORT(dev->gpio_loc);
        pwm_pin = AF_TIMER0_CC1_PIN(dev->gpio_loc);
        /* Route the location of PWM channel to output pin */
        TIMER0->ROUTELOC0 = (TIMER0->ROUTELOC0 & ~_TIMER_ROUTELOC0_CC1LOC_MASK)
        | (dev->gpio_loc << _TIMER_ROUTELOC0_CC1LOC_SHIFT);
        TIMER0->ROUTEPEN |= TIMER_ROUTEPEN_CC1PEN; /*CC Channel 1 pin Enable */
        PRINTF("PWM-ARCH: Timer0 CC channel 1 is configured.\n");
      break;
      
      case 2:
        pwm_port = AF_TIMER0_CC2_PORT(dev->gpio_loc);
        pwm_pin = AF_TIMER0_CC2_PIN(dev->gpio_loc);
        /* Route the location of PWM channel to output pin */
        TIMER0->ROUTELOC0 = (TIMER0->ROUTELOC0 & ~_TIMER_ROUTELOC0_CC2LOC_MASK)
        | (dev->gpio_loc << _TIMER_ROUTELOC0_CC2LOC_SHIFT);
        TIMER0->ROUTEPEN |= TIMER_ROUTEPEN_CC2PEN; /*CC Channel 2 pin Enable */
        PRINTF("PWM-ARCH: Timer0 CC channel 2 is configured.\n");
      break;
      
      default:
        PRINTF("PWM-ARCH: Timer0 CC channel: %u not Available!!!\n", dev->cc_channel);
        return PWM_STATUS_INVALID_CHANNEL;
      break;
    }
    /* Set pin as output pin */
    if(dev->pwm_active_logic == ENABLE_ACTIVE_LOW) {
      GPIO_PinModeSet(pwm_port, pwm_pin, gpioModePushPull, 1);
    } else {
      GPIO_PinModeSet(pwm_port, pwm_pin, gpioModePushPull, 0);
    }
    /* Configure timer if it has not been configured. this will also start the timer */
    if(pwm_config_timer0 == NULL) {
      TIMER_Init(TIMER0, &timerInit);
      PRINTF("PWM-ARCH: Timer0 is configured and started.\n");
    }
    /* assign the config to this timer */
    pwm_config_timer0 = dev->config;
  }
#endif /* TIMER0 */
#ifdef TIMER1
  else if(dev->config->timer_per == TIMER1) {
    /* configure the timer only if the config is different than what is already set */
    if(dev->config != pwm_config_timer1) {
      if(pwm_config_timer1 != NULL) {
        /* timer is set in different config already by some other device! */
        PRINTF("PWM-ARCH: Timer1 is already in use by some other device!!!\n");
        return PWM_STATUS_BUSY;
      }
      /* enable the timer clock frequency */
      CMU_ClockEnable(cmuClock_TIMER1, true);
      /* Set Top Value, TOP=(F_HFPER/(2^PRESC x F_PWM))-1 here PRESC value=0 */
      TIMER_TopSet(TIMER1, counter_top);
      PRINTF("PWM-ARCH: Set Timer1 Top: %lu\n", counter_top);
    } else {
      PRINTF("PWM-ARCH: Timer1 is already configured with same config.\n");
    }
    /* Configure CC channel 0 */
    TIMER_InitCC(TIMER1, dev->cc_channel, &timerCCInit);
    /* Set compare value for PWM compare channel (CC) to compare value. */
    TIMER_CompareBufSet(TIMER1, dev->cc_channel, compare_value);
    switch (dev->cc_channel) {
      case 0:
        pwm_port = AF_TIMER1_CC0_PORT(dev->gpio_loc);
        pwm_pin = AF_TIMER1_CC0_PIN(dev->gpio_loc);
        /* Route the location of PWM channel to output pin */
        TIMER1->ROUTELOC0 = (TIMER1->ROUTELOC0 & ~_TIMER_ROUTELOC0_CC0LOC_MASK)
        | (dev->gpio_loc << _TIMER_ROUTELOC0_CC0LOC_SHIFT);
        TIMER1->ROUTEPEN |= TIMER_ROUTEPEN_CC0PEN; /*CC Channel 0 pin Enable */
        PRINTF("PWM-ARCH: Timer1 CC channel 0 is configured.\n");
      break;
      
      case 1:
        pwm_port = AF_TIMER1_CC1_PORT(dev->gpio_loc);
        pwm_pin = AF_TIMER1_CC1_PIN(dev->gpio_loc);
        /* Route the location of PWM channel to output pin */
        TIMER1->ROUTELOC0 = (TIMER1->ROUTELOC0 & ~_TIMER_ROUTELOC0_CC1LOC_MASK)
        | (dev->gpio_loc << _TIMER_ROUTELOC0_CC1LOC_SHIFT);
        TIMER1->ROUTEPEN |= TIMER_ROUTEPEN_CC1PEN; /*CC Channel 1 pin Enable */
        PRINTF("PWM-ARCH: Timer1 CC channel 1 is configured.\n");
      break;
      
      case 2:
        pwm_port = AF_TIMER1_CC2_PORT(dev->gpio_loc);
        pwm_pin = AF_TIMER1_CC2_PIN(dev->gpio_loc);
        /* Route the location of PWM channel to output pin */
        TIMER1->ROUTELOC0 = (TIMER1->ROUTELOC0 & ~_TIMER_ROUTELOC0_CC2LOC_MASK)
        | (dev->gpio_loc << _TIMER_ROUTELOC0_CC2LOC_SHIFT);
        TIMER1->ROUTEPEN |= TIMER_ROUTEPEN_CC2PEN; /*CC Channel 2 pin Enable */
        PRINTF("PWM-ARCH: Timer1 CC channel 2 is configured.\n");
      break;
      
      case 3:
        pwm_port = AF_TIMER1_CC3_PORT(dev->gpio_loc);
        pwm_pin = AF_TIMER1_CC3_PIN(dev->gpio_loc);
        /* Route the location of PWM channel to output pin */
        TIMER1->ROUTELOC0 = (TIMER1->ROUTELOC0 & ~_TIMER_ROUTELOC0_CC3LOC_MASK)
        | (dev->gpio_loc << _TIMER_ROUTELOC0_CC3LOC_SHIFT);
        TIMER1->ROUTEPEN |= TIMER_ROUTEPEN_CC3PEN; /*CC Channel 0 pin Enable */
        PRINTF("PWM-ARCH: Timer1 CC channel 3 is configured.\n");
      break;
      
      default:
        PRINTF("PWM-ARCH: TIMER1 CC channel:%u not Available!!!\n", dev->cc_channel);
        return PWM_STATUS_INVALID_CHANNEL;
      break;
    }
    /* Set pin as output pin */
    if(dev->pwm_active_logic == ENABLE_ACTIVE_LOW) {
      GPIO_PinModeSet(pwm_port, pwm_pin, gpioModePushPull, 1);
    } else {
      GPIO_PinModeSet(pwm_port, pwm_pin, gpioModePushPull, 0);
    }
    /* Configure timer. this will also start the timer */
    if(pwm_config_timer1 == NULL) {
      TIMER_Init(TIMER1, &timerInit);
      PRINTF("PWM-ARCH: Timer1 is configured and started.\n");
    }
    pwm_config_timer1 = dev->config;
  }
#endif /* TIMER1 */
  else {
    PRINTF("PWM-ARCH: Given timer is not implemented !!!\n");
    return PWM_STATUS_INVALID_TIMER;
  }
  return PWM_STATUS_OK;
}
/*---------------------------------------------------------------------------*/
pwm_status_t 
pwm_arch_reset(pwm_dev_t *dev)
{
  GPIO_Port_TypeDef pwm_port;
  uint8_t pwm_pin;
  if(!TIMER_REF_VALID(dev->config->timer_per)) {
    PRINTF("PWM-ARCH: Given timer is not supported by this arch !!!\n");
    return PWM_STATUS_INVALID_TIMER;
  }
#ifdef TIMER0
  if(dev->config->timer_per == TIMER0) {
    TIMER_Reset(TIMER0);
    switch (dev->cc_channel) {
      case 0:
        pwm_port = AF_TIMER0_CC0_PORT(dev->gpio_loc);
        pwm_pin = AF_TIMER0_CC0_PIN(dev->gpio_loc);
        TIMER0->ROUTEPEN &= ~TIMER_ROUTEPEN_CC0PEN;
        TIMER0->ROUTELOC0 &= ~_TIMER_ROUTELOC0_CC0LOC_MASK;
      break;

      case 1:
        pwm_port = AF_TIMER0_CC0_PORT(dev->gpio_loc);
        pwm_pin = AF_TIMER0_CC0_PIN(dev->gpio_loc);
        TIMER0->ROUTEPEN &= ~TIMER_ROUTEPEN_CC1PEN;
        TIMER0->ROUTELOC0 &= ~_TIMER_ROUTELOC0_CC1LOC_MASK;
      break;

      case 2:
        pwm_port = AF_TIMER0_CC0_PORT(dev->gpio_loc);
        pwm_pin = AF_TIMER0_CC0_PIN(dev->gpio_loc);
        TIMER0->ROUTEPEN &= ~TIMER_ROUTEPEN_CC2PEN;
        TIMER0->ROUTELOC0 &= ~_TIMER_ROUTELOC0_CC2LOC_MASK;
      break;
      
      default:
        PRINTF("PWM-ARCH: TIMER1 CC channel:%u not Available!!!\n", dev->cc_channel);
        return PWM_STATUS_INVALID_CHANNEL;
      break;
    }
    CMU_ClockEnable(cmuClock_TIMER0, false);
    if(dev->pwm_active_logic == ENABLE_ACTIVE_LOW) {
      GPIO_PinModeSet(pwm_port, pwm_pin, gpioModeDisabled, 1);
    } else {
      GPIO_PinModeSet(pwm_port, pwm_pin, gpioModeDisabled, 0);
    }
    pwm_config_timer0 = NULL;
  }
#endif /* TIMER0 */
#ifdef TIMER1
  else if(dev->config->timer_per == TIMER1) {
    TIMER_Reset(TIMER1);
    switch(dev->cc_channel) {
      case 0:
        pwm_port = AF_TIMER0_CC0_PORT(dev->gpio_loc);
        pwm_pin = AF_TIMER0_CC0_PIN(dev->gpio_loc);
        TIMER1->ROUTEPEN &= ~TIMER_ROUTEPEN_CC0PEN;
        TIMER1->ROUTELOC0 &= ~_TIMER_ROUTELOC0_CC0LOC_MASK;
      break;

      case 1:
        pwm_port = AF_TIMER0_CC0_PORT(dev->gpio_loc);
        pwm_pin = AF_TIMER0_CC0_PIN(dev->gpio_loc);
        TIMER1->ROUTEPEN &= ~TIMER_ROUTEPEN_CC1PEN;
        TIMER1->ROUTELOC0 &= ~_TIMER_ROUTELOC0_CC1LOC_MASK;
      break;
      
      case 2:
        pwm_port = AF_TIMER0_CC0_PORT(dev->gpio_loc);
        pwm_pin = AF_TIMER0_CC0_PIN(dev->gpio_loc);
        TIMER1->ROUTEPEN &= ~TIMER_ROUTEPEN_CC2PEN;
        TIMER1->ROUTELOC0 &= ~_TIMER_ROUTELOC0_CC2LOC_MASK;
      break;
      
      case 3:
        pwm_port = AF_TIMER0_CC0_PORT(dev->gpio_loc);
        pwm_pin = AF_TIMER0_CC0_PIN(dev->gpio_loc);
        TIMER1->ROUTEPEN &= ~TIMER_ROUTEPEN_CC3PEN;
        TIMER1->ROUTELOC0 &= ~_TIMER_ROUTELOC0_CC3LOC_MASK;
      break;
      
      default:
        PRINTF("PWM-ARCH: TIMER1 CC channel:%u not Available!!!\n", dev->cc_channel);
        return PWM_STATUS_INVALID_CHANNEL;
      break;
    }
    CMU_ClockEnable(cmuClock_TIMER1, false);
    if(dev->pwm_active_logic == ENABLE_ACTIVE_LOW) {
      GPIO_PinModeSet(pwm_port, pwm_pin, gpioModeDisabled, 1);
    } else {
      GPIO_PinModeSet(pwm_port, pwm_pin, gpioModeDisabled, 0);
    }
    pwm_config_timer1 = NULL;
  }
#endif /* TIMER1 */
  return PWM_STATUS_OK;
}
/*---------------------------------------------------------------------------*/
pwm_status_t 
pwm_arch_set_duty_cycle(pwm_dev_t *dev)
{
  uint32_t compare_value;
  uint32_t counter_top;
  if(!TIMER_REF_VALID(dev->config->timer_per)) {
    PRINTF("PWM-ARCH: Given timer is not supported by this arch !!!\n");
    return PWM_STATUS_INVALID_TIMER;
  }
  counter_top = CMU_ClockFreqGet(cmuClock_HFPER) / dev->config->freq_hz - 1;
  if(dev->duty_cycle_100x > 10000) {
   dev->duty_cycle_100x = 10000; /* limit to 100% duty cycle */
  }
  compare_value = (counter_top * dev->duty_cycle_100x) / 10000;
#ifdef TIMER0
  if(dev->config->timer_per == TIMER0) {
    /* Set compare value for PWM compare channel (CC) to compare value. */
    TIMER_CompareBufSet(TIMER0, dev->cc_channel, compare_value);
    PRINTF("PWM-ARCH: Set Timer0 CC channel %u compare value: %lu\n", dev->cc_channel, compare_value);
  }
#endif /* TIMER0 */
#ifdef TIMER1
  else if(dev->config->timer_per == TIMER1){
    TIMER_CompareBufSet(TIMER1, dev->cc_channel, compare_value);
    PRINTF("PWM-ARCH: Set Timer1 CC channel %u compare value: %lu\n", dev->cc_channel, compare_value);
  }
#endif /* TIMER1 */
  else {
    PRINTF("PWM-ARCH: Timer is not implemented!!!\n");
    return PWM_STATUS_INVALID_TIMER;
  }
  return PWM_STATUS_OK;
}
/*---------------------------------------------------------------------------*/
pwm_status_t
pwm_arch_enable_device(pwm_dev_t *dev, uint8_t on_off)
{
  if(dev->dev_enable != NULL) {
    if(on_off == PWM_DEVICE_ENABLE) {
      if(dev->dev_enable->logic == ENABLE_ACTIVE_LOW) {
        GPIO_PinModeSet(dev->dev_enable->port, dev->dev_enable->pin, gpioModePushPull, 0);
        PRINTF("PWM-ARCH: PWM device enabled active low...\n");
      } else {
        GPIO_PinModeSet(dev->dev_enable->port, dev->dev_enable->pin, gpioModePushPull, 1);
        PRINTF("PWM-ARCH: PWM device enabled active high...\n");
      }
    } else {
      if(dev->dev_enable->logic == ENABLE_ACTIVE_LOW) {
        GPIO_PinModeSet(dev->dev_enable->port, dev->dev_enable->pin, gpioModeDisabled, 1);
        PRINTF("PWM-ARCH: PWM device disabled high...\n");
      } else {
        GPIO_PinModeSet(dev->dev_enable->port, dev->dev_enable->pin, gpioModeDisabled, 0);
        PRINTF("PWM-ARCH: PWM device disabled low...\n");
      }
    }
  }
  return PWM_STATUS_OK;
}
/*---------------------------------------------------------------------------*/
