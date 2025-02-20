#ifndef _EFR32_ADC_ARCH_H_
#define _EFR32_ADC_ARCH_H_
#include <em_adc.h>

#define EFR32_ADC_CLOCK_HZ    1000000     /* ADC frequency in hertz */
#define ADC_DEFAULT_SAMPLES   10          /* Number of ADC samples taken in single measurement */
#define ADC_RESOLUTION        4096        /* 12 bit ADC resolution */

typedef enum {
  ADC_REF_MV_1250 = 1250,
  ADC_REF_MV_2500 = 2500,
  ADC_REF_MV_5000 = 5000
} adc_internal_ref_mv_t;

/**
 * @brief function reads raw adc value in single mode for given input parameters
 * 
 * @param adc_ref_mv  ADC reference voltage to select
 * @param pos_input   ADC positive signal input
 * @param neg_input   ADC negative signal input. for single ended conversion this must be adcNegSelVSS
 * @param samples     Number of samples to take for averaged output
 * @return uint32_t    function returns raw ADC value
 */
uint32_t adc_read_single(ADC_Ref_TypeDef adc_ref_mv, ADC_PosSel_TypeDef pos_input,
          ADC_NegSel_TypeDef neg_input, uint16_t samples);

/**
 * @brief function reads ADC value and converts it in millivolts based on 
 *        selected internal ADC Vref. This function can be used only when
 *        using internal reference voltage as ADC reference voltage.
 * 
 * @param adc_ref_mv  ADC internal reference voltage
 * @param pos_input   ADC positive input pin
 * @param neg_input   ADC negative input pin. for single ended conversions this must be adcNegSelVSS
 * @param samples     number of samples to take and averaged
 * @return uint32_t    function returns millivolts on the ADC input pin with selected internal ADC Vref
 */
uint32_t adc_read_millivolts(adc_internal_ref_mv_t adc_ref_mv, 
                            ADC_PosSel_TypeDef pos_input,
                            ADC_NegSel_TypeDef neg_input,
                            uint16_t samples);
/**
 * @brief function initializes the ADC module by selecting auxiliary clock source
 *        with EFR32_ADC_CLOCK_HZ clock rate. It also calibrates the ADC for 
 *        2.5V reference voltage
 */
void adc_init(void);
#endif  /* _EFR32_ADC_ARCH_H_ */
