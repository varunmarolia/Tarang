/**
 * @file guart.c
 * @author Varun Marolia
 * @brief This driver implements generic UART. The driver implements method for 
 *        debug printf output and also implements an ISR callback method for RX 
 *        data reception in a circular buffer. In the event if the debug output 
 *        is on SWO pin, the UART TX will not use the printf function.
 * 
 * @copyright Copyright (c) 2025 Varun Marolia
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

static guart_t *debug_uart = NULL;  /* pointer to debug uart. Will be used by stdout */
/*---------------------------------------------------------------------------*/
void
guart_init(guart_t *uart)
{
  if(uart != NULL) {
    serial_dev_bus_acquire(uart->guart_dev);
    uart->rx_buff_head = 0;
    uart->rx_buff_tail = 0;
    uart->new_line_buff_index = GUART_RX_BUFFER_SIZE;
  }
}
/*---------------------------------------------------------------------------*/
void
guart_send_data(guart_t *uart, const uint8_t *data, uint16_t bytes)
{
  serial_bus_status_t bus_status;
  if(uart != NULL) {
    bus_status = serial_arch_transfer(uart->guart_dev, data, bytes, NULL, 0);
    if(bus_status != BUS_OK) {
      PRINTF("GUART: failed to send data, error:%u\n", bus_status);
    } 
  }
}
/*---------------------------------------------------------------------------*/
uint16_t
guart_read_data(guart_t *uart, uint8_t *data)
{
  uint16_t read_bytes = 0;
  uint16_t i;
  if(uart != NULL) {
    if(uart->rx_buff_head != uart->rx_buff_tail) {
      i = uart->rx_buff_tail;
      while(i != uart->rx_buff_head) {
        data[read_bytes++] = uart->rx_buff[i];
        i = (i + 1) % GUART_RX_BUFFER_SIZE;
      }
      ATOMIC_SECTION(
        uart->rx_buff_tail = uart->rx_buff_head;
        uart->new_line_buff_index = GUART_RX_BUFFER_SIZE;  /* reset new line index */
      );
    }
  }
  return read_bytes;
}
/*---------------------------------------------------------------------------*/
uint16_t
guart_read_line(guart_t *uart, uint8_t *data)
{
  uint16_t read_bytes = 0;
  uint16_t i;
  /* check if pointer is null */
  if(uart !=NULL ) {
    /* Is there data in buffer ? */
    if(uart->rx_buff_head != uart->rx_buff_tail) {
      /* does the data have new line character ? */
      if(uart->new_line_buff_index < GUART_RX_BUFFER_SIZE) {
        for(i = uart->rx_buff_tail; i != uart->new_line_buff_index; i = (i + 1) % GUART_RX_BUFFER_SIZE) {
          if(data[read_bytes] == '\n') {
            break;  /* new line character found */
          }
          data[read_bytes++] = uart->rx_buff[i];  /* copy data from tail to new line index */
        }
        data[read_bytes++] = uart->rx_buff[i];  /* copy new line character */
        ATOMIC_SECTION(
          uart->rx_buff_tail = (uart->rx_buff_tail + read_bytes) % GUART_RX_BUFFER_SIZE;
          /* reset the new line index */
          if(i == uart->new_line_buff_index) {
            uart->new_line_buff_index = GUART_RX_BUFFER_SIZE;  /* reset new line index */
          }
        );
      }
    }
  }
  return read_bytes;
}
/*---------------------------------------------------------------------------*/
void
guart_debug_input_handler(uint8_t data)
{
  if(debug_uart != NULL) {
    debug_uart->rx_buff[debug_uart->rx_buff_head] = data;
    /* If received new line character. update the buffer index to latest new line index */
    if(data == '\n') {
      debug_uart->new_line_buff_index = debug_uart->rx_buff_head;
    }
    debug_uart->rx_buff_head = (debug_uart->rx_buff_head + 1) % GUART_RX_BUFFER_SIZE;
  }
}
/*---------------------------------------------------------------------------*/
void
guart_puts(guart_t *uart, const char *str)
{
  guart_send_data(uart, (uint8_t *)str, strlen(str));
}
/*---------------------------------------------------------------------------*/
void
guart_debug_puts(const char *str)
{
  guart_send_data(debug_uart, (uint8_t *)str, strlen(str));
}
/*---------------------------------------------------------------------------*/
void
guart_set_debug_stdo(guart_t *uart)
{
  debug_uart = uart;
  serial_dev_set_input_handler(uart->guart_dev, guart_debug_input_handler);
}
/*---------------------------------------------------------------------------*/
#ifndef USE_SWO_DEBUG
#undef stdio_put_char_bw
void 
stdio_put_char_bw(char c)
{
  if(debug_uart != NULL) {
    guart_send_data(debug_uart, (uint8_t *)&c, 1);
  }
}
#endif /* USE_SWO_DEBUG */
/*---------------------------------------------------------------------------*/
