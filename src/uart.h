#ifndef UART_H
#define UART_H

#include <stdint.h>

#include "types.h"

void uart_init(uint16_t baudrate);

void uart_putc(char c);
void uart_puts(const char *s);

void uart_buf_putc(char c);
void uart_buf_puts(const char *s);
void uart_buf_puti8(uint8_t i);
void uart_buf_puti16(uint16_t i);
void uart_buf_putf(float f);

boolean uart_getc(char *c);

#endif
