/**
 * @file  crc8.c
 * @author Varun Marolia
 * @brief This file implements function bodies to calculate CRC8
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

#include "crc8.h"

/*---------------------------------------------------------------------------*/
uint8_t
crc8_calc_buff(const crc8_cfg_t *cfg, uint8_t *buff, uint8_t buff_length)
{
  uint8_t crc = cfg->intial_remainder;
  uint8_t i = 0;
  uint8_t bit;
  if(buff_length && buff) {
    for(i = 0; i < buff_length; i++) {
      crc ^= buff[i];
      for(bit = 8; bit > 0; --bit) {
        if(crc & 0x80) {
          crc = (crc << 1) ^ cfg->polynomial;
        } else {
          crc = (crc << 1);
        }
      }
    }
  }
return (crc ^ cfg->final_xor_value);
}
/*---------------------------------------------------------------------------*/
