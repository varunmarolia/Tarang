
/**
 * @file      Akashvani1-board.c
 * @author    Varun Marolia
 * @brief     Akashvani is a dual band 2.4GHz and 868/915 MHz radio module based on 
 *            EFR32BG13P732F512GM48 and CC1200
 *
 *  @copyright Copyright (c) 2025 Varun Marolia
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

#include "board-akashvani1.h"
#include <em_device.h>
#include <em_emu.h>
#include <em_cmu.h>
#include "serial-dev.h"

serial_bus_t spi_bus_0 = {
  .lock = false,
  .current_dev = NULL,
  .config = {
    .data_in_loc = CC1200_MISO_LOC,
    .data_out_loc = CC1200_MOSI_LOC,
    .clk_loc = CC1200_CLK_LOC,
    .type = BUS_TYPE_SPI,
    .SPI_UART_USARTx = CC1200_SPI_USART,
    .I2Cx = NULL,
    .clock_mode = usartClockMode0,
    .msb_first = true
  }
};
/*---------------------------------------------------------------------------*/
void 
akashvani1_module_init(void)
{
  /* Disable DCDC power regulator */
  EMU_DCDCPowerOff();
  /* tune and select external clock oscillator*/
  CMU_HFXOInit_TypeDef hfxoInit = CMU_HFXOINIT_DEFAULT;
  hfxoInit.ctuneStartup = BOARD_HFXO_CTUNE_VALUE_SU;
  hfxoInit.ctuneSteadyState = BOARD_HFXO_CTUNE_VALUE_SS;
  /* Initialize the HFXO with default parameters before using it*/
  CMU_HFXOInit(&hfxoInit);
  /* Select the external oscillator as HF clock source. This will also enable the XOSC */
  CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFXO);
  /* if the external Oscillator is enabled and selected, disable the internal HFRC oscillator */
  if(CMU_ClockSelectGet(cmuClock_HF) == cmuSelect_HFXO) {
    CMU_OscillatorEnable(cmuOsc_HFRCO, false, false);
  }
}
