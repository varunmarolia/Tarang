/**
 * @file adc-dev.c
 * @author Varun Marolia
 * @brief This file contains methods for using ADC devices.
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
adc_dev_read_microvolts(adc_dev_t *dev, uint32_t *microvolts)
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
  *microvolts = adc_arch_read_microvolts(dev);
  /* Disable the device */
  if(dev->adc_dev_enable) {
    adc_arch_dev_enable(dev->adc_dev_enable, ADC_DEV_DISABLE);
  }
  return ADC_OK;
}
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
