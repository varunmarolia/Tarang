#include "board.h"
#include "serial-dev.h"

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

serial_bus_t debug_uart_bus = {
  .lock = false,
  .current_dev = NULL,
  .config = {
    .data_in_loc = DEBUG_UART_RX_LOC,
    .data_out_loc = DEBUG_UART_TX_LOC,
    .rts_loc = DEBUG_UART_RTS_LOC,
    .cts_loc = DEBUG_UART_CTS_LOC,
    .uart_mode = UART_MODE_TX_RX_FLOW,
    .uart_input_handler = NULL,
    .type = BUS_TYPE_UART,
    .I2Cx = NULL,
    .SPI_UART_USARTx = DEBUG_USART_BASE
  }
};
/*---------------------------------------------------------------------------*/
void
board_init() {
    akashvani1_module_init();
}
