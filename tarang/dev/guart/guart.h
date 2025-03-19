#ifndef __GENERIC_UART_H__
#define __GENERIC_UART_H__
#include <stdint.h>
#include "serial-dev.h"

#define GUART_RX_BUFFER_SIZE                128
#define GUART_MAX_USB_UART_EFR32_BAUDRATE   1200000         /* 1.2Mbps, Actual speed 120000 Bps. This baud produces integer value for USART_CLKDIV on EFR32 */
#define GUART_MAX_STD_UART_BAUDRATE         921600          /* 921.6Kbps, Actual speed 92160 Bps */

typedef struct guart {
  uint8_t rx_buff[GUART_RX_BUFFER_SIZE];    /* Circular queue buffer */
  volatile uint16_t rx_buff_head;           /* buffer head */
  volatile uint16_t rx_buff_tail;           /* buffer tail */
  uint16_t new_line_buff_index;             /* new line character index in the rx_buff of last received byte */
  serial_dev_t *guart_dev;                  /* pointer to a generic uart device */
  void (*rx_handler)(unsigned char c);      /* function pointer to Rx interrupt handler for this uart device */
} guart_t;

void guart_send_data(guart_t *uart, const uint8_t *data, uint16_t bytes);
uint16_t guart_read_data(guart_t *uart, uint8_t *data);
uint16_t guart_read_line(guart_t *uart, uint8_t *data);
void guart_init(guart_t *uart);
void guart_debug_input_handler(uint8_t data);
void guart_puts(guart_t *uart, const char *str);
void guart_debug_puts(const char *str);
void guart_set_debug_stdo(guart_t *uart);
#endif /* __GENERIC_UART_H__ */
