#ifndef _ADC_DEV_H_
#define _ADC_DEV_H_

#include "adc-arch.h"
#include "common-arch.h"

typedef enum adc_status {
  ADC_OK = 0,
  ADC_BUSY = 1,
  ADC_INVALID = 2
} adc_status_t;

typedef struct adc_config {
  const ADC_PosSel_TypeDef pos_input;
  const ADC_NegSel_TypeDef neg_input;
  const ADC_Ref_TypeDef adc_ref_mv;
  ADC_TypeDef *adc_peripheral;
} adc_config_t;

typedef struct adc_dev {
  const uint16_t adc_avg_samples;         /* number of samples to take and average */
  const uint32_t power_up_delay_ms;       /* power up delay in case the end device needs to be enabled before measurment, will be used by adc_dev_enable if assigned */
  /* arch specific variables */
  adc_config_t *adc_config;
  gpio_config_t *adc_dev_enable;
} adc_dev_t; 

adc_status_t adc_dev_read_microvolts(adc_dev_t *dev, uint32_t *microvolts);
adc_status_t adc_dev_read_single(adc_dev_t *dev, uint32_t *adc_value);
adc_status_t adc_dev_init(adc_dev_t *dev);
/*********** Arch specific functions **************/
/**
 * @brief function reads raw adc value in single mode for given input parameters
 * 
 * @param  dev         Pointer to the ADC device strucutre
 * @return uint32_t    function returns raw ADC value
 */
uint32_t adc_arch_read_single(adc_dev_t *dev);

/**
 * @brief function reads ADC value and converts it in microvolts based on 
 *        selected internal ADC Vref. This function can be used only when
 *        using internal reference voltage as ADC reference voltage.
 * 
 * @param  dev         Pointer to the ADC device structure
 * @return uint32_t    function returns microvolts on the ADC input pin with selected internal ADC Vref
 */
uint32_t adc_arch_read_microvolts(adc_dev_t *dev);
/**
 * @brief function initializes the ADC module by selecting auxiliary clock source
 *        with EFR32_ADC_CLOCK_HZ clock rate. It also calibrates the ADC for 
 *        2.5V reference voltage
 */
void adc_arch_init(ADC_TypeDef *adc_peripheral);
/**
 * @brief function enable or disable the ADC device by enabling or disabling the
 *        GPIO pin connected to the ADC device
 * 
 * @param cs      Pointer to the GPIO configuration structure
 * @param on_off  ADC_DEV_ENABLE or ADC_DEV_DISABLE the ADC device
 */
void adc_arch_dev_enable(gpio_config_t *cs, uint8_t on_off);

#endif  /* _ADC_DEV_H_ */
