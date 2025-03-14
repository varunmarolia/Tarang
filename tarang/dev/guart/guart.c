#include "board.h"
#include "guart.h"
#include "serial-dev.h"
#include "stdio-op.h"
#include <string.h>
#include "atomic.h"

#define DEBUG_GUART 0     /**< Set this to 1 for debug printf output */
#if DEBUG_GUART
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)      /**< Replace printf with nothing */
#endif /* DEBUG_GUART */

#ifndef GENERIC_UART_BUS
#error "GENERIC_UART_BUS must be defined to use this piece of code"
#endif  /* GENERIC_UART_BUS */

uint8_t rx_buff[GUART_RX_BUFFER_SIZE];
volatile uint16_t rx_buff_head = 0;
volatile uint16_t rx_buff_tail = 0;
extern serial_bus_t GENERIC_UART_BUS;
serial_dev_t guart_dev = {
  .bus = &GENERIC_UART_BUS,
  .speed_hz = SERIAL_UART_DEFAUT_BAUDRATE,
  .address = 0,
  .timeout_ms = 0,
  .power_up_delay_ms = 0,
  .cs_config = NULL
};
/*---------------------------------------------------------------------------*/
void
guart_init(void)
{
  serial_dev_bus_acquire(&guart_dev);
  serial_dev_set_input_handler(&guart_dev, guart_input_handler);
}
/*---------------------------------------------------------------------------*/
void
guart_send_data(const uint8_t *data, uint16_t bytes)
{
  serial_bus_status_t bus_status;
  bus_status = serial_arch_transfer(&guart_dev, data, bytes, NULL, 0);
  if(bus_status != BUS_OK) {
    PRINTF("GUART: failed to send data, error:%u\n", bus_status);
  } 
}
/*---------------------------------------------------------------------------*/
uint16_t
guart_read_data(uint8_t *data)
{
  uint16_t read_bytes = 0;
  uint16_t i;
  if(rx_buff_head != rx_buff_tail) {
    i = rx_buff_tail;
    while(i != rx_buff_head) {
      data[read_bytes++] = rx_buff[i];
      i = (i + 1) % GUART_RX_BUFFER_SIZE;
    }
    ATOMIC_SECTION(
      rx_buff_tail = rx_buff_head;
    );
  }
  return read_bytes;
}
/*---------------------------------------------------------------------------*/
uint16_t
guart_read_line(uint8_t *data)
{
  uint16_t read_bytes = 0;
  uint16_t i;
  if(rx_buff_head != rx_buff_tail) {
    for(i = rx_buff_tail; i != rx_buff_head; i = (i + 1) % GUART_RX_BUFFER_SIZE) {
      data[read_bytes++] = rx_buff[i];
      if(rx_buff[i] == '\n') {
        break;
      }
    }
    ATOMIC_SECTION(
      rx_buff_tail = (rx_buff_tail + read_bytes) % GUART_RX_BUFFER_SIZE;
    );
  }
  return read_bytes;
}
/*---------------------------------------------------------------------------*/
void
guart_input_handler(uint8_t data)
{
  rx_buff[rx_buff_head] = data;
  rx_buff_head = (rx_buff_head + 1) % GUART_RX_BUFFER_SIZE;
}
/*---------------------------------------------------------------------------*/
void
guart_puts(const char *str)
{
  guart_send_data((uint8_t *)str, strlen(str));
}
/*---------------------------------------------------------------------------*/
#ifndef USE_SWO_DEBUG
#undef stdio_put_char_bw
void 
stdio_put_char_bw(char c)
{
  guart_send_data((uint8_t *)&c, 1);
}
#endif /* USE_SWO_DEBUG */
/*---------------------------------------------------------------------------*/
