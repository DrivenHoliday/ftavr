#include "uart.h"

#include <stdlib.h>
#include <stdio.h>

#include <avr/io.h>
#include <avr/interrupt.h>

#define UART_RINGBUF_SIZE (32)

static char uart_ringbuf[UART_RINGBUF_SIZE];
static uint8_t uart_rpos = 0;
static uint8_t uart_wpos = 0;

ISR(USART_UDRE_vect)
{
    if(uart_ringbuf[uart_rpos])
    {
        uart_putc(uart_ringbuf[uart_rpos]);
        uart_ringbuf[uart_rpos] = '\0';

        if(!(++uart_rpos%UART_RINGBUF_SIZE))
            uart_rpos = 0;
    }
    else
    {
        UCSRB &= ~(1<<UDRIE);
    }
}

void uart_init(uint16_t baudrate)
{
    uint8_t n=0;
    for(;n<UART_RINGBUF_SIZE;++n) uart_ringbuf[n]='\0';

    if(baudrate & 0x8000)
    {
        UCSRA = (1<<U2X);  //Enable 2x speed
        baudrate &= ~0x8000;
    }

    UBRRH = (uint8_t)(baudrate>>8);
    UBRRL = (uint8_t) baudrate;

    UCSRB |= (1<<TXEN)|(1<<UDRIE);  // UART TX einschalten
    UCSRC |= (1<<URSEL)|(1<<UCSZ1)|(1<<UCSZ0);  // Asynchron 8N1
}

void uart_putc(char c)
{
    while(!(UCSRA & (1<<UDRE)));
    UDR = c;
}

void uart_puts(const char *s)
{
    while (*s) uart_putc(*s++);
}

void uart_buf_putc(char c)
{
    uart_ringbuf[uart_wpos++] = c;
    if(!(uart_wpos%UART_RINGBUF_SIZE))
        uart_wpos = 0;
    UCSRB |= (1<<UDRIE);
}

void uart_buf_puts(const char *s)
{
    while(*s) uart_buf_putc(*s++);
}

#define PUTIBUF_SIZE (20)
static char putibuf[PUTIBUF_SIZE];

void uart_buf_puti8(uint8_t i)
{
    uart_buf_puts(itoa(i,putibuf,10));
}

void uart_buf_puti16(uint16_t i)
{
    uart_buf_puts(itoa(i,putibuf,10));
}

void uart_buf_putf(float f)
{
    sprintf(putibuf,"%f",f);
    uart_buf_puts(putibuf);
}
