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
 
 #ifndef BOARD_ADC_PER_BASE
 #define BOARD_ADC_PER_BASE    ADC0
 #endif  /* BOARD_ADC_PER_BASE */
 
 #define ADC_DEBUG 0
 #if ADC_DEBUG
 #include <stdio.h>
 #undef PRINTF
 #define PRINTF(...) printf(__VA_ARGS__)
 #else  /* ADC_DEBUG */
 /**< Replace printf with nothing */
 #define PRINTF(...)
 #endif /* ADC_DEBUG */
 /*---------------------------------------------------------------------------*/
 /* calibration must be called after ADC and clk init */
 static void
 calibrate_ADC()
 {
   uint8_t mid;
   uint32_t cal, sample;
   ADC_InitSingle_TypeDef singleInit = ADC_INITSINGLE_DEFAULT;
 
   /* Initalize single measurement of GND signal */
   singleInit.reference = adcRef2V5;
   singleInit.posSel    = adcPosSelVSS;
   singleInit.negSel    = adcNegSelVSS;
   singleInit.acqTime   = adcAcqTime16;
   singleInit.fifoOverwrite = true;
 
   ADC_InitSingle(ADC0, &singleInit);
 
   /* Single ended offset calibration, scan from lower ADC results */
   mid = 15;
   ADC0->CAL |= ADC_CAL_CALEN;
   ADC0->SINGLEFIFOCLEAR = ADC_SINGLEFIFOCLEAR_SINGLEFIFOCLEAR;
 
   for(mid = 15; mid >= 0; mid--) {
     /* Write to calibration register */
     cal = ADC0->CAL & ~(_ADC_CAL_SINGLEOFFSET_MASK);
     cal |= (uint8_t)(mid) << _ADC_CAL_SINGLEOFFSET_SHIFT;
     ADC0->CAL = cal;
 
     /* Start ADC single conversion */
     ADC_Start(ADC0, adcStartSingle);
     /* wait for result*/
     while(!(ADC0->IF & ADC_IF_SINGLE));
 
     sample = ADC_DataSingleGet(ADC0);
     /* break when we read 0 or we're at max offset */
     if(!(sample)) {
       break;
     }
   }
 
   /* exit calibration mode */
   ADC0->CAL &= ~(ADC_CAL_CALEN);
 }
 /*---------------------------------------------------------------------------*/
 void 
 adc_init(void)
 {
   ADC_Init_TypeDef ADCInit = ADC_INIT_DEFAULT;
 
   /* Enable ADC peripheral clock */
   CMU_ClockEnable(cmuClock_ADC0, true);
   /* Select AUXHFRCO for ADC ASYNC mode so that ADC can run on EM2 */
   CMU->ADCCTRL = CMU_ADCCTRL_ADC0CLKSEL_AUXHFRCO;
   /* find number of clk cycles to count 1us using HFPER clk */
   ADCInit.timebase = ADC_TimebaseCalc(0);
   /* find ADC pre-scaler value to set the given ADC frequency */
   ADCInit.prescale = ADC_PrescaleCalc(EFR32_ADC_CLOCK_HZ, 0);
   /* ADC shut down after each conversion. 5us warm up time is used for each conversion */
   ADCInit.warmUpMode = adcWarmupNormal;
   /* ADC clock is enabled only during ADC conversion */
   ADCInit.em2ClockConfig = adcEm2ClockOnDemand;
   ADC_Init(BOARD_ADC_PER_BASE, &ADCInit);
   /* calibrate ADC offset values for 2.5V internal reference*/
   calibrate_ADC();
 }
 /*---------------------------------------------------------------------------*/
 uint32_t
 adc_read_single(ADC_Ref_TypeDef adc_ref, ADC_PosSel_TypeDef pos_input,
           ADC_NegSel_TypeDef neg_input, uint16_t samples)
 {
   uint16_t i;
   uint32_t sum_adc_reading = 0;
   ADC_InitSingle_TypeDef adc_init_single = ADC_INITSINGLE_DEFAULT;
   
   /* verify inputs */
   if(!samples) {
     samples = ADC_DEFAULT_SAMPLES;  /* load default ADC samples */
   }
   /* setup ADC in single measurement mode */
   adc_init_single.reference = adc_ref;
   adc_init_single.posSel = pos_input;
   adc_init_single.negSel = neg_input;
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
   ADC_InitSingle(BOARD_ADC_PER_BASE, &adc_init_single);
 
   /* take ADC samples */
   for(i = 0; i < samples; i++) {
     ADC_Start(BOARD_ADC_PER_BASE, adcStartSingle);
     while(!(BOARD_ADC_PER_BASE->IF & ADC_IF_SINGLE));
     sum_adc_reading += ADC_DataSingleGet(BOARD_ADC_PER_BASE) & 0x0000FFFF;
   }
   /* find sample average */
   sum_adc_reading /= samples;
 #if ADC_DEBUG
   PRINTF("ADC reading:%lu\n", sum_adc_reading);
 #endif /* ADC_DEBUG */
   return sum_adc_reading;
 }
 /*---------------------------------------------------------------------------*/
 uint32_t
 adc_read_millivolts(adc_internal_ref_mv_t adc_ref_mv, 
                     ADC_PosSel_TypeDef pos_input,
                     ADC_NegSel_TypeDef neg_input,
                     uint16_t samples)
 {
   uint32_t adc_reading;
   ADC_Ref_TypeDef adc_ref = adcRef2V5;  /* default adc ref value is 2.5v */
   /* select right ref value based on selected Reference voltage */
   if(adc_ref_mv == ADC_REF_MV_5000) {
     adc_ref = adcRef5V;
   }
   if(adc_ref_mv == ADC_REF_MV_1250) {
     adc_ref = adcRef1V25;
   }
   adc_reading = adc_read_single(adc_ref, pos_input, neg_input, samples);
   /* convert the reading into millivolt for selected internal reference voltage */
   adc_reading *= adc_ref_mv;
   adc_reading /= ADC_RESOLUTION;
   return adc_reading;
 }
 /*---------------------------------------------------------------------------*/
 