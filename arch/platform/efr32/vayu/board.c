#include "board.h"
#include "serial-dev.h"

serial_bus_t i2c_bus_0 = {
  .lock = false,
  .config = {
    .data_in_loc = I2C_BUS_DATA_LOC,
    .data_out_loc = I2C_BUS_DATA_LOC,
    .clk_loc = I2C_BUS_CLOCK_LOC,
    .type = BUS_TYPE_I2C,
    .I2Cx = I2C0
  }
};
/*---------------------------------------------------------------------------*/
void
board_init() {
    akashvani1_module_init();
}
