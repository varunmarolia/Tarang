#include "pwm-arch.h"
#include <em_cmu.h>
#include <em_timer.h>

#define PWM_ARCH 0     /**< Set this to 1 for debug printf output */
#if PWM_ARCH
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)      /**< Replace printf with nothing */
#endif /* PWM_ARCH */

/*---------------------------------------------------------------------------*/
void 
pwm_arch_init(pwm_dev_t *dev)
{
  GPIO_Port_TypeDef pwm_port;
  uint8_t pwm_pin;
  uint32_t counter_top = CMU_ClockFreqGet(cmuClock_HFPER) / dev->config->freq_hz - 1;
  uint32_t compare_value = (counter_top * dev->duty_cycle_100x) / 10000;
  if(!TIMER_REF_VALID(dev->config->timer_per)) {
    PRINTF("PWM-ARCH: Given timer is not supported by this arch !!!\n");
    return;
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
  if(dev->active_logic == ENABLE_ACTIVE_LOW) {
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
  if(dev->config->timer_per == TIMER0) {
    CMU_ClockEnable(cmuClock_TIMER0, true);
    /* Configure CC channel */
    TIMER_InitCC(TIMER0, dev->cc_channel, &timerCCInit);
    /* Set Top Value, TOP=(F_HFPER/(2^PRESC x F_PWM))-1 here PRESC value = 0 */
    TIMER_TopSet(TIMER0, counter_top);
    /* Set compare value for PWM compare channel (CC) to compare value. */
    TIMER_CompareBufSet(TIMER0, dev->cc_channel, compare_value);
    switch (dev->cc_channel) {
      case 0:
        pwm_port = AF_TIMER0_CC0_PORT(dev->gpio_loc);
        pwm_pin = AF_TIMER0_CC0_PIN(dev->gpio_loc);
        /* Route the location of PWM channel to output pin */
        TIMER0->ROUTELOC0 = (TIMER0->ROUTELOC0 & ~_TIMER_ROUTELOC0_CC0LOC_MASK)
        | (dev->gpio_loc << _TIMER_ROUTELOC0_CC0LOC_SHIFT);
        TIMER0->ROUTEPEN |= TIMER_ROUTEPEN_CC0PEN; /*CC Channel 0 pin Enable */
      break;
      
      case 1:
        pwm_port = AF_TIMER0_CC1_PORT(dev->gpio_loc);
        pwm_pin = AF_TIMER0_CC1_PIN(dev->gpio_loc);
        /* Route the location of PWM channel to output pin */
        TIMER0->ROUTELOC0 = (TIMER0->ROUTELOC0 & ~_TIMER_ROUTELOC0_CC1LOC_MASK)
        | (dev->gpio_loc << _TIMER_ROUTELOC0_CC1LOC_SHIFT);
        TIMER0->ROUTEPEN |= TIMER_ROUTEPEN_CC1PEN; /*CC Channel 1 pin Enable */
      break;
      
      case 2:
        pwm_port = AF_TIMER0_CC2_PORT(dev->gpio_loc);
        pwm_pin = AF_TIMER0_CC2_PIN(dev->gpio_loc);
        /* Route the location of PWM channel to output pin */
        TIMER0->ROUTELOC0 = (TIMER0->ROUTELOC0 & ~_TIMER_ROUTELOC0_CC2LOC_MASK)
        | (dev->gpio_loc << _TIMER_ROUTELOC0_CC2LOC_SHIFT);
        TIMER0->ROUTEPEN |= TIMER_ROUTEPEN_CC2PEN; /*CC Channel 2 pin Enable */
      break;
      
      default:
        PRINTF("PWM-ARCH: Timer0 CC channel: %u not Available!!!\n", dev->cc_channel);
        return;
      break;
    }
    /* Set pin as output pin */
    if(dev->active_logic == ENABLE_ACTIVE_LOW) {
      GPIO_PinModeSet(pwm_port, pwm_pin, gpioModePushPull, 1);
    } else {
      GPIO_PinModeSet(pwm_port, pwm_pin, gpioModePushPull, 0);
    }
    /* Configure timer. this will also start the timer */
    TIMER_Init(TIMER0, &timerInit);
  } 
#ifdef TIMER1
  else if(dev->config->timer_per == TIMER1) {
    CMU_ClockEnable(cmuClock_TIMER1, true);
    /* Configure CC channel 0 */
    TIMER_InitCC(TIMER1, dev->cc_channel, &timerCCInit);
    /* Set Top Value, TOP=(F_HFPER/(2^PRESC x F_PWM))-1 here PRESC value=0 */
    TIMER_TopSet(TIMER1, counter_top);
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
      break;
      
      case 1:
        pwm_port = AF_TIMER1_CC1_PORT(dev->gpio_loc);
        pwm_pin = AF_TIMER1_CC1_PIN(dev->gpio_loc);
        /* Route the location of PWM channel to output pin */
        TIMER1->ROUTELOC0 = (TIMER1->ROUTELOC0 & ~_TIMER_ROUTELOC0_CC1LOC_MASK)
        | (dev->gpio_loc << _TIMER_ROUTELOC0_CC1LOC_SHIFT);
        TIMER1->ROUTEPEN |= TIMER_ROUTEPEN_CC1PEN; /*CC Channel 1 pin Enable */
      break;
      
      case 2:
        pwm_port = AF_TIMER1_CC2_PORT(dev->gpio_loc);
        pwm_pin = AF_TIMER1_CC2_PIN(dev->gpio_loc);
        /* Route the location of PWM channel to output pin */
        TIMER1->ROUTELOC0 = (TIMER1->ROUTELOC0 & ~_TIMER_ROUTELOC0_CC2LOC_MASK)
        | (dev->gpio_loc << _TIMER_ROUTELOC0_CC2LOC_SHIFT);
        TIMER1->ROUTEPEN |= TIMER_ROUTEPEN_CC2PEN; /*CC Channel 2 pin Enable */
      break;
      
      case 3:
        pwm_port = AF_TIMER1_CC3_PORT(dev->gpio_loc);
        pwm_pin = AF_TIMER1_CC3_PIN(dev->gpio_loc);
        /* Route the location of PWM channel to output pin */
        TIMER1->ROUTELOC0 = (TIMER1->ROUTELOC0 & ~_TIMER_ROUTELOC0_CC3LOC_MASK)
        | (dev->gpio_loc << _TIMER_ROUTELOC0_CC3LOC_SHIFT);
        TIMER1->ROUTEPEN |= TIMER_ROUTEPEN_CC3PEN; /*CC Channel 0 pin Enable */
      break;
      
      default:
        PRINTF("PWM-ARCH: TIMER1 CC channel:%u not Available!!!\n", dev->cc_channel);
        return;
      break;
    }
    /* Set pin as output pin */
    if(dev->active_logic == ENABLE_ACTIVE_LOW) {
      GPIO_PinModeSet(pwm_port, pwm_pin, gpioModePushPull, 1);
    } else {
      GPIO_PinModeSet(pwm_port, pwm_pin, gpioModePushPull, 0);
    }
    /* Configure timer. this will also start the timer */
    TIMER_Init(TIMER1, &timerInit);
  }
#endif /* TIMER1 */
  else {
    PRINTF("PWM-ARCH: Given timer is not implemented !!!\n");
    return;
  }
}
/*---------------------------------------------------------------------------*/
void 
pwm_arch_reset(pwm_dev_t *dev)
{
  GPIO_Port_TypeDef pwm_port;
  uint8_t pwm_pin;
  if(!TIMER_REF_VALID(dev->config->timer_per)) {
    PRINTF("PWM-ARCH: Given timer is not supported by this arch !!!\n");
    return;
  }
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
        return;
      break;
    }
    CMU_ClockEnable(cmuClock_TIMER0, true);
    if(dev->active_logic == ENABLE_ACTIVE_LOW) {
      GPIO_PinModeSet(pwm_port, pwm_pin, gpioModeDisabled, 1);
    } else {
      GPIO_PinModeSet(pwm_port, pwm_pin, gpioModeDisabled, 0);
    }
  }
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
        return;
      break;
    }
    CMU_ClockEnable(cmuClock_TIMER1, true);
    if(dev->active_logic == ENABLE_ACTIVE_LOW) {
      GPIO_PinModeSet(pwm_port, pwm_pin, gpioModeDisabled, 1);
    } else {
      GPIO_PinModeSet(pwm_port, pwm_pin, gpioModeDisabled, 0);
    }
  }
#endif /* TIMER1 */
}
/*---------------------------------------------------------------------------*/
void 
pwm_arch_set_duty_cycle(pwm_dev_t *dev)
{
  uint32_t compare_value;
  uint32_t counter_top;
  if(!TIMER_REF_VALID(dev->config->timer_per)) {
    PRINTF("PWM-ARCH: Given timer is not supported by this arch !!!\n");
    return;
  }
  counter_top = CMU_ClockFreqGet(cmuClock_HFPER) / dev->config->freq_hz - 1;
  compare_value = (counter_top * dev->duty_cycle_100x) / 10000;
  if(dev->config->timer_per == TIMER0) {
    /* Set compare value for PWM compare channel (CC) to compare value. */
    TIMER_CompareBufSet(TIMER0, dev->cc_channel, compare_value);
  }
#ifdef TIMER1
  else if(dev->config->timer_per == TIMER1){
    TIMER_CompareBufSet(TIMER1, dev->cc_channel, compare_value);
  }
#endif /* TIMER1 */
  else {
    PRINTF("PWM-ARCH: Timer is not implemented!!!\n");
  }
}
/*---------------------------------------------------------------------------*/
