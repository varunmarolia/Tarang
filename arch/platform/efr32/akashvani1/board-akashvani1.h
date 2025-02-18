/***************************************************************************************
 * @file      board-akashvani1.h
 * @brief     Akashvani is a dual band 2.4GHz and 868/915 MHz radio module based on 
 *            EFR32BG13P732F512GM48 and CC1200
 **************************************************************************************/

#ifndef BOARD_AKASHVANI1_H_
#define BOARD_AKASHVANI1_H_

#ifndef EFR32BG13P732F512GM48        /**< This is defined in CFLAGS must be defined before incliuding em_device.h */
#define EFR32BG13P732F512GM48
#endif
#include "em_device.h"               /**< This is required for adding the cpu header */

/*
 * calculate loading capacitance value 
 * --- from EFR32BG13P reference manual ---
 * TUNING = ((desiredTotalLoadCap × 2 - Min(CLFXO_T)) / CLFXO_TS)
 * --- from EFR32BG13P part datasheet ---
 * Min CLFXO_T = 8 pf and CLFXO_TS = SSLFXO = 0.25 pf and 
 * Min CHFXO_T = 9 pf and CHFXO_TS = SSHFXO = 0.04 pf
 * --- from datasheet of LFXO (VXM9-1EE-10-38M4000000) ---
 * required loading capacitance = 12.5 pf
 * Effective Loading capacitance on HFXTAL_N and HFXTAL_P 
 * = CL + Cstray (Cstray usually 0.5 pf)
 * 
 * LFXO TUNING = (without stray capacitance) = ((12.5 x 2 - 8) / 0.25) = 68
 * LFXO TUNING (with stray capacitance) = (((12.5 - 0.5) x 2 - 8) / 0.25) = 64
 * 
 * HFXO TUNING = (without stray capacitance)= ((10 x 2 - 9) / 0.04) = 275
 * HFXO TUNING (with stray capacitance)= (((10 - 0.5) x 2 - 9) / 0.04) = 250
 * @note the values defined below are derived experimentally and may differ
 * from the values calculated above.
 */
#define BOARD_LFXO_CTUNE_VALUE      72  /* Ctune value for LFXO. @note Value yet to verify */
#define BOARD_HFXO_CTUNE_VALUE_SU  160  /* Ctune startup of HFXO. @note Value yet to verify */
#define BOARD_HFXO_CTUNE_VALUE_SS  360  /* Ctune steady state of HFXO. @note Value yet to verify */

/* Pin/SPI configuration for CC1200 included in akashvani1 module */
#define CC1200_SPI_BUS          spi_bus_0
#define CC1200_SPI_USART        USART2

#define CC1200_MISO_PORT        gpioPortF
#define CC1200_MISO_PIN         5 /* PF5 */
#define CC1200_MISO_LOC         _USART_ROUTELOC0_RXLOC_LOC17

#define CC1200_MOSI_PORT        gpioPortF
#define CC1200_MOSI_PIN         7 /* PF7 */
#define CC1200_MOSI_LOC         _USART_ROUTELOC0_TXLOC_LOC20

#define CC1200_CLK_PORT         gpioPortF
#define CC1200_CLK_PIN          6 /* PF6 */
#define CC1200_CLK_LOC          _USART_ROUTELOC0_CLKLOC_LOC17

#define CC1200_CS_PORT          gpioPortF
#define CC1200_CS_PIN           4 /* PF4 */
#define CC1200_CS_CONFIG        {CC1200_CS_PORT, CC1200_CS_PIN, ENABLE_ACTIVE_LOW}

#define CC1200_GPIO_0_PORT      gpioPortC
#define CC1200_GPIO_0_PIN       8

#define CC1200_GPIO_2_PORT      gpioPortC
#define CC1200_GPIO_2_PIN       7

#define CC1200_RESET_PORT       gpioPortC
#define CC1200_RESET_PIN        9

#define BOARD_TOTAL_RADIOS      2
#define BOARD_HAS_2G4HZ_RADIO   1
#define BOARD_HAS_SUBGHZ_RADIO  1

void akashvani1_module_init(void);

#endif /* BOARD_AKASHVANI1_H_ */
