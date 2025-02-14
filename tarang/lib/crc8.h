#ifndef CRC8_H_
#define CRC8_H_

#include <stdint.h>

#define CRC_NAME                 "CRC-8"


typedef struct crc8_config
{
uint8_t polynomial;
uint8_t intial_remainder;
uint8_t final_xor_value;
} crc8_cfg_t;

/*!
* \fn     uint8_t crc8_calc_buff(const crc8_cfg_t cfg, uint8_t *buff, uint8_t buff_size)
* \brief  Function calculates 8 bit crc values for given byte.
* \param  crc8_cfg Pointer to CRC8 config strcture holding values like
*         polynomial to use intial remainder to use and final xor value.
* \param  buff Pointer to the data buffer.
* \param  buff_size Number of data bytes in buffer. This can not be more than 255.
* \return Function returns 8 bit CRC value for given data buffer of given size.
*/
uint8_t crc8_calc_buff(const crc8_cfg_t *cfg, uint8_t *buff, uint8_t buff_length);

#endif
