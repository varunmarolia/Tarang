#ifndef _TARANG_FLASH_H_
#define _TARANG_FLASH_H_
#include <stdint.h>

typedef enum flash_status {
  FLASH_STATUS_OK = 0,
  FLASH_STATUS_INVALID_ADDRESS = -1,
  FLASH_STATUS_FLASH_LOCKED = -2,
  FLASH_STATUS_TIMEOUT = -3,
} flash_status_t;

flash_status_t flash_lock();
flash_status_t flash_unlock();
flash_status_t flash_erase_sector(uint32_t address);
flash_status_t flash_write_word(uint32_t address, uint32_t data);
#endif /* _TARANG_FLASH_H_ */
