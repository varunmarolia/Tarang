#include "adc-dev.h"
#include "clock.h"
/*---------------------------------------------------------------------------*/
adc_status_t 
adc_dev_init(adc_dev_t *dev)
{
  if(dev == NULL || dev->adc_config == NULL) {
    return ADC_INVALID;
  }
  adc_arch_init(dev->adc_config->adc_peripheral);
  return ADC_OK;
}
/*---------------------------------------------------------------------------*/
adc_status_t
adc_dev_read_single(adc_dev_t *dev, uint32_t *adc_value)
{
  if(dev == NULL || dev->adc_config == NULL) {
    return ADC_INVALID;
  }
  /* check if device needs to be enabled */
  if(dev->adc_dev_enable) {
    adc_arch_dev_enable(dev->adc_dev_enable, ADC_DEV_ENABLE);
    /* wait for power up delay time */
    if(dev->power_up_delay_ms) {
      clock_wait_ms(dev->power_up_delay_ms);
    }
  }
  /* read ADC */
  *adc_value = adc_arch_read_single(dev);
  /* Disable the device */
  if(dev->adc_dev_enable) {
    adc_arch_dev_enable(dev->adc_dev_enable, ADC_DEV_DISABLE);
  }
  return ADC_OK;
}
/*---------------------------------------------------------------------------*/
adc_status_t
adc_dev_read_millivolts(adc_dev_t *dev, uint32_t *millivolts)
{
  if(dev == NULL || dev->adc_config == NULL) {
    return ADC_INVALID;
  }
  /* check if device needs to be enabled */
  if(dev->adc_dev_enable) {
    adc_arch_dev_enable(dev->adc_dev_enable, ADC_DEV_ENABLE);
    /* wait for power up delay time */
    if(dev->power_up_delay_ms) {
      clock_wait_ms(dev->power_up_delay_ms);
    }
  }
  *millivolts = adc_arch_read_millivolts(dev);
  /* Disable the device */
  if(dev->adc_dev_enable) {
    adc_arch_dev_enable(dev->adc_dev_enable, ADC_DEV_DISABLE);
  }
  return ADC_OK;
}
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
