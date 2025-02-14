/*!
*\file  crc8.c
*\brief This file implements function bodies to calculate CRC8
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
