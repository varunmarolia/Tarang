/*! 
 * @file    adc-arch.c
 * @brief   This file implements methods to initialize and operate
 *          ADC peripheral of EFR32 MCU.
 *          Following are the assumption taken for using the ADC
 *          - ADC peripheral ADC0 is used as default peripheral for taking readings
 *          - ADC resolution is fixed and set to 12 bit
 *          - ADC peripheral clock is fixed and set to 1MHz
 *          - Only internal (known value) ADC reference voltages are supported
 *          Calibration
 *          - According to ref manual and AN0021, the calibration values for the 1.25 V reference
 *            are written to the ADC calibration register after reset. If the EFM32 Gecko is
 *            operated at different temperatures or with external references, calibration should 
 *            be performed.
 */

 #include "adc-arch.h"
 #include <em_cmu.h>
 #include <em_gpio.h>
 #include "board.h"

 #define ADC_DEBUG 0
 #if ADC_DEBUG
 #include <stdio.h>
 #undef PRINTF
 #define PRINTF(...) printf(__VA_ARGS__)
 #else  /* ADC_DEBUG */
 /**< Replace printf with nothing */
 #define PRINTF(...)
 #endif /* ADC_DEBUG */
 static bool adc_initialized = false;
 /*---------------------------------------------------------------------------*/
 /* calibration must be called after ADC and clk init */
 static void
 calibrate_ADC(ADC_TypeDef *adc_peripheral)
 {
   uint8_t mid;
   uint32_t cal, sample;
   ADC_InitSingle_TypeDef singleInit = ADC_INITSINGLE_DEFAULT;
 
   /* Initialize single measurement of GND signal */
   singleInit.reference = adcRef2V5;
   singleInit.posSel    = adcPosSelVSS;
   singleInit.negSel    = adcNegSelVSS;
   singleInit.acqTime   = adcAcqTime16;
   singleInit.fifoOverwrite = true;
 
   ADC_InitSingle(adc_peripheral, &singleInit);
 
   /* Single ended offset calibration, scan from lower ADC results */
   mid = 15;
   adc_peripheral->CAL |= ADC_CAL_CALEN;
   adc_peripheral->SINGLEFIFOCLEAR = ADC_SINGLEFIFOCLEAR_SINGLEFIFOCLEAR;
 
   for(mid = 15; mid >= 0; mid--) {
     /* Write to calibration register */
     cal = adc_peripheral->CAL & ~(_ADC_CAL_SINGLEOFFSET_MASK);
     cal |= (uint8_t)(mid) << _ADC_CAL_SINGLEOFFSET_SHIFT;
     adc_peripheral->CAL = cal;
 
     /* Start ADC single conversion */
     ADC_Start(adc_peripheral, adcStartSingle);
     /* wait for result*/
     while(!(adc_peripheral->IF & ADC_IF_SINGLE));
 
     sample = ADC_DataSingleGet(adc_peripheral);
     /* break when we read 0 or we're at max offset */
     if(!(sample)) {
       break;
     }
   }
   /* exit calibration mode */
   adc_peripheral->CAL &= ~(ADC_CAL_CALEN);
 }
 /*---------------------------------------------------------------------------*/
 void 
 adc_arch_init(ADC_TypeDef *adc_peripheral)
 {
  ADC_Init_TypeDef ADCInit = ADC_INIT_DEFAULT;
  if(adc_initialized == false) {
    /* Enable ADC peripheral clock */
    if(adc_peripheral == ADC0) {
      CMU_ClockEnable(cmuClock_ADC0, true);
    }
  #ifdef ADC1 
    else if(adc_peripheral == ADC1) {
      CMU_ClockEnable(cmuClock_ADC1, true);
    }
  #endif  /* ADC1 */
    else {
      PRINTF("ADC peripheral not supported\n");
      return;
    }
    /* Select AUXHFRCO for ADC ASYNC mode so that ADC can run on EM2 */
    CMU->ADCCTRL = CMU_ADCCTRL_ADC0CLKSEL_AUXHFRCO;
    /* find number of clk cycles to count 1us using HFPER clk */
    ADCInit.timebase = ADC_TimebaseCalc(0);
    /* find ADC pre-scaler value to set the given ADC frequency */
    ADCInit.prescale = ADC_PrescaleCalc(EFR32_ADC_DEFAULT_CLOCK_HZ, 0);
    /* ADC shut down after each conversion. 5us warm up time is used for each conversion */
    ADCInit.warmUpMode = adcWarmupNormal;
    /* ADC clock is enabled only during ADC conversion */
    ADCInit.em2ClockConfig = adcEm2ClockOnDemand;
    ADC_Init(adc_peripheral, &ADCInit);
    /* calibrate ADC offset values for 2.5V internal reference*/
    calibrate_ADC(adc_peripheral);
    adc_initialized = true;
  }
 }
 /*---------------------------------------------------------------------------*/
 uint32_t
 adc_arch_read_single(adc_dev_t *dev)
 {
   uint16_t i, samples;
   uint32_t sum_adc_reading = 0;
   ADC_InitSingle_TypeDef adc_init_single = ADC_INITSINGLE_DEFAULT;
   
   if(dev == NULL || dev->adc_config->adc_peripheral == NULL) {
    PRINTF("ADC arch: ADC device NULL\n");
    return 0;
   }
   if(adc_initialized == false) {
    PRINTF("ADC arch: ADC must be initialized before use. Initializing...\n ");
    adc_arch_init(dev->adc_config->adc_peripheral);
   }
   /* verify inputs */
   if(!dev->adc_avg_samples) {
     samples = ADC_DEFAULT_SAMPLES;  /* load default ADC samples */
   } else {
     samples = dev->adc_avg_samples;
   }
   /* setup ADC in single measurement mode */
   adc_init_single.reference = dev->adc_config->adc_ref_mv;
   adc_init_single.posSel = dev->adc_config->pos_input; 
   adc_init_single.negSel = dev->adc_config->neg_input;
   /*
    * 16 adc clk cycles acquisition time. 
    * according to reference manual the minimum acquisition time 
    * for sampling at 1Msps and typical input loading is 187.5ns.
    * For high impedance sources it takes more to charge the internal
    * sampling capacitor.
    * T(conv) = (T(acq) + (N(adc res bits) + 1) * T(adc_clk_sar)) x OVSRSEL
    * @ 1MHz ADC clock
    * T(conv) = 16 us + 13 * 1us * 1 = 29 us
    * Total ADC time = ADCREF_WARMUPTIME + T(conv) = 29us + 5us = 34 us / sample
    */
   adc_init_single.acqTime = adcAcqTime16; 
   ADC_InitSingle(dev->adc_config->adc_peripheral, &adc_init_single);
 
   /* take ADC samples */
   for(i = 0; i < samples; i++) {
     ADC_Start(dev->adc_config->adc_peripheral, adcStartSingle);
     while(!(dev->adc_config->adc_peripheral->IF & ADC_IF_SINGLE));
     sum_adc_reading += ADC_DataSingleGet(dev->adc_config->adc_peripheral) & 0x0000FFFF;
   }
   /* find sample average */
   sum_adc_reading /= samples;
   PRINTF("ADC arch: reading:%lu\n", sum_adc_reading);
   return sum_adc_reading;
 }
 /*---------------------------------------------------------------------------*/
 uint32_t
 adc_arch_read_microvolts(adc_dev_t *dev)
 {
   uint32_t adc_reading;
   uint32_t adc_ref_mv = 2500;
   uint64_t uv = 0;
   /* select right ref value based on selected Reference voltage */
   adc_reading = adc_arch_read_single(dev);
   /* convert the reading into millivolt for selected internal reference voltage */
   if(dev->adc_config->adc_ref_mv == adcRef1V25) {
     adc_ref_mv = 1250;
   } else if(dev->adc_config->adc_ref_mv == adcRef2V5) {
     adc_ref_mv = 2500;
   } else if(dev->adc_config->adc_ref_mv == adcRef5V) {
     adc_ref_mv = 5000;
   }
#ifdef BOARD_ADC_REF_mVDD
   else if(dev->adc_config->adc_ref_mv == adcRefVDD) {
     adc_ref_mv = BOARD_ADC_REF_mVDD;
   }
#endif  /* BOARD_ADC_REF_mVDD */
    else {
      PRINTF("ADC reference voltage not supported\n");
      return 0;
    }
   uv = adc_reading;
   uv *= adc_ref_mv;
   uv *= 1000;
   uv /= ADC_RESOLUTION;
   PRINTF("ADC arch: microvolt:%lu ADC ref:%lu\n", (uint32_t)uv, adc_ref_mv);
   return (uint32_t)uv;
 }
 /*---------------------------------------------------------------------------*/
 void 
 adc_arch_dev_enable(gpio_config_t *cs, uint8_t on_off)
 {
   if(cs != NULL) {
    if(on_off == ADC_DEV_ENABLE) {
      if(cs->logic == ENABLE_ACTIVE_LOW) {
        GPIO_PinModeSet(cs->port, cs->pin, gpioModeWiredAnd, 0);
        PRINTF("ADC arch: GPIO pin set low\n");
      } else {
        GPIO_PinModeSet(cs->port, cs->pin, gpioModePushPull, 1);
      }
    } else {
       GPIO_PinModeSet(cs->port, cs->pin, gpioModeDisabled, 0);
       PRINTF("ADC arch: GPIO pin disabled\n");
    }
  }
 }
 /*---------------------------------------------------------------------------*/
/**
 * @todo - Implement ADC read method for differential mode input signals.
 *       - Implement interrupt based ADC conversion.
 *       - Implement ADC peripheral lock/unlock mechanism
 * 
 */