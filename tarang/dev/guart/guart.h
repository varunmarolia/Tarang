#ifndef __GENERIC_UART_H__
#define __GENERIC_UART_H__
#include <stdint.h>

#define GUART_RX_BUFFER_SIZE 128
#define GUART_MAX_USB_UART_EFR32_BAUDRATE 1200000   /* 1.2Mbps, Actual speed 120000 Bps. This baud prodcues integer value for USART_CLKDIV on EFR32 */
#define GUART_MAX_STD_UART_BAUDRATE 921600          /* 921.6Kbps, Actual speed 92160 Bps */

#endif /* __GENERIC_UART_H__ */
