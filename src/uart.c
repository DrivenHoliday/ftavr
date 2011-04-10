#include "uart.h"

#include <stdlib.h>
#include <stdio.h>

#include <avr/io.h>
#include <avr/interrupt.h>

#include <util/atomic.h>

#include "ringbuf.h"

#define UART_RINGBUF_SIZE 128

RINGBUF_DEFINE(char, UART_RINGBUF_SIZE);
RINGBUF_CREATE(char, UART_RINGBUF_SIZE, out_buf, static volatile);
RINGBUF_CREATE(char, UART_RINGBUF_SIZE, in_buf, static volatile);

ISR(USART_UDRE_vect)
{
    char c;
    RINGBUF_READ(out_buf, c);
    
    if(c)
    {
        uart_putc(c);
        RINGBUF_MARK(out_buf, '\0');
    }
    else
    {
        UCSRB &= ~(1<<UDRIE);       
    }
}

ISR(USART_RXC_vect)
{
    RINGBUF_WRITE(in_buf, UDR);
}

boolean uart_getc(char *c)
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        RINGBUF_READ(in_buf, *c);
    
        if(*c)
        {
            RINGBUF_MARK(in_buf, '\0');
            return TRUE;
        }
        else
        {
            return FALSE;       
        }
    }
}

void uart_init(uint16_t baudrate)
{
    RINGBUF_INIT(out_buf);
    RINGBUF_INIT(in_buf);
    
    if(baudrate & 0x8000)
    {
        UCSRA = (1<<U2X);  /* Enable 2x speed */
        baudrate &= ~0x8000;
    }

    UBRRH = (uint8_t)(baudrate>>8);
    UBRRL = (uint8_t) baudrate;

    UCSRB |= (1<<TXEN)|(1<<RXEN); /* activate UART TX/RX */
    UCSRB |= (1<<UDRIE)|(1<<RXCIE); /* enable interrupts */
    UCSRC |= (1<<URSEL)|(1<<UCSZ1)|(1<<UCSZ0);  /* asynchronous 8N1 */
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
    RINGBUF_WRITE(out_buf, c);
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
