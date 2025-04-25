#include "dev/common/flash.h"
#include <em_msc.h>
#include <em_system.h>

#define EFR32_FLASH_PAGE_MASK (0xFFFFFFFF - FLASH_PAGE_SIZE  + 1) /* flash page size of 2KB */
/*---------------------------------------------------------------------------*/
static flash_status_t
msc_status_to_flash_status(MSC_Status_TypeDef status) {
  switch(status) {
    case mscReturnOk:
      return FLASH_STATUS_OK;
    case mscReturnInvalidAddr:
      return FLASH_STATUS_INVALID_ADDRESS;
    case mscReturnLocked:
      return FLASH_STATUS_FLASH_LOCKED;
    case mscReturnTimeOut:
      return FLASH_STATUS_TIMEOUT;
    default:
      return FLASH_STATUS_TIMEOUT;
  }
}
/*---------------------------------------------------------------------------*/
flash_status_t flash_lock(void) {
  /* All em_msc.c functions lock and unlock the flash pages */
  return FLASH_STATUS_OK;
}
/*---------------------------------------------------------------------------*/
flash_status_t flash_unlock(void) {
  /* All em_msc.c functions lock and unlock the flash pages */
  return FLASH_STATUS_OK;
}
/*---------------------------------------------------------------------------*/
flash_status_t flash_erase_sector(uint32_t address) {
  if(address < FLASH_SIZE) {
    return FLASH_STATUS_INVALID_ADDRESS;
  }
  /* make sure the address is beginning of a flash page */
  address &= EFR32_FLASH_PAGE_MASK;
  return msc_status_to_flash_status(MSC_ErasePage((uint32_t *)address)); /* check the status of erase operation */
}
/*---------------------------------------------------------------------------*/
flash_status_t flash_write_word(uint32_t address, uint32_t data) {
  /* check if address is multiplication of word size */
  if((address & 0x3) != 0 && address < FLASH_SIZE) {
    return FLASH_STATUS_INVALID_ADDRESS; /* address is not word aligned */
  }
  return msc_status_to_flash_status(MSC_WriteWord((uint32_t *)address, &data, 4)); /* write the data to flash */
}
