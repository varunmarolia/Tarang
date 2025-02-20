#include "board.h"
#include "serial-dev.h"
#include "em_usart.h"

serial_bus_t i2c_bus_0 = {
  .lock = false,
  .current_dev = NULL,
  .config = {
    .data_in_loc = I2C_BUS_DATA_LOC,
    .data_out_loc = I2C_BUS_DATA_LOC,
    .clk_loc = I2C_BUS_CLOCK_LOC,
    .type = BUS_TYPE_I2C,
    .I2Cx = I2C0,
    .SPI_UART_USARTx = NULL
  }
};

serial_bus_t generic_uart_bus = {
  .lock = false,
  .current_dev = NULL,
  .config = {
    .data_in_loc = UART_MCU_RX_LOC,
    .data_out_loc = UART_MCU_TX_LOC,
    .cts_loc = UART_MCU_CTS_LOC,
    .rts_loc = UART_MCU_RTS_LOC,
    .parity_mode = USART_FRAME_PARITY_NONE,
    .stop_bits = USART_FRAME_STOPBITS_ONE,
    .uart_mode = UART_MODE_TX_RX,
    .input_handler = NULL,
    .output_handler = NULL,
    .type = BUS_TYPE_UART,
    .I2Cx = NULL,
    .SPI_UART_USARTx = UART_USART
  }
};
/*---------------------------------------------------------------------------*/
void
board_init(void) {
  /* Initialize the RF module. This will select XTALs */
  akashvani1_module_init();
  /* Enable clock for GPIO */
  CMU_ClockEnable(cmuClock_GPIO, true);
  /* Configure LED_PORT pin LED_PIN (User LED) as push/pull outputs */
  GPIO_PinModeSet(LED_SYS_GREEN_PORT,         /* Port */
                  LED_SYS_GREEN_PIN,          /* Pin */
                  gpioModePushPull,           /* Mode */
                  1 );                        /* Output value */
  GPIO_PinModeSet(LED_SYS_YELLOW_PORT,        /* Port */
                  LED_SYS_YELLOW_PIN,         /* Pin */
                  gpioModePushPull,           /* Mode */
                  1 );                        /* Output value */
  GPIO_PinModeSet(LED_MODE_GREEN_PORT,        /* Port */
                  LED_MODE_GREEN_PIN,         /* Pin */
                  gpioModePushPull,           /* Mode */
                  1 );                        /* Output value */
  GPIO_PinModeSet(LED_MODE_YELLOW_PORT,       /* Port */
                  LED_MODE_YELLOW_PIN,        /* Pin */
                  gpioModePushPull,           /* Mode */
                  1 );                        /* Output value */
  /* Initialize buttons */
  GPIO_PinModeSet(RESET_PUSH_BUTTON_PORT, 
                  RESET_PUSH_BUTTON_PIN, 
                  gpioModeInput, 
                  1);
  GPIO_PinModeSet(MODE_PUSH_BUTTON_PORT,
                  MODE_PUSH_BUTTON_PIN,
                  gpioModeInput,
                  1);
}
