#ifndef _ADC_ARCH_H_
#define _ADC_ARCH_H_
#include <em_adc.h>
#include "common-arch.h"
#include <stddef.h>

#define EFR32_ADC_DEFAULT_CLOCK_HZ    1000000                           /* ADC frequency in hertz */
#define ADC_DEFAULT_SAMPLES           10                                /* Number of ADC samples taken in single measurement */
#define ADC_RESOLTION_BITS            12                                /* 12 bit ADC resolution */ 
#define ADC_RESOLUTION                (0x00001 << ADC_RESOLTION_BITS)   /* ADC resolution */
#define ADC_DEV_ENABLE                1                                 /* enable the ADC device */
#define ADC_DEV_DISABLE               0                                 /* disable the ADC device */

typedef struct adc_config {
  const ADC_PosSel_TypeDef pos_input;
  const ADC_NegSel_TypeDef neg_input;
  const ADC_Ref_TypeDef adc_ref_mv;
  ADC_TypeDef *adc_peripheral;
} adc_config_t;

#endif  /* _ADC_ARCH_H_ */
