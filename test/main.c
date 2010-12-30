#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>

#include <avr/delay.h>

#define BAUD 19200
#include <util/setbaud.h>

#include "uart.h"

#include <stdint.h>

int main(void)
{
    uint8_t last[2] = {0,0};

    uart_init(UBRR_VALUE);

    DDRD = 0x0;
    PORTD = 0x0;

    DDRC = 0xff;
    PORTC = 0x0;
    
    sei();

    while(1)
    {
        if(PIND & (1<<PC2))
        {
            if(!last[0])
            {
                uart_buf_puts("Tor 1\n");
                last[0] = 1;
            }
        }
        else
            last[0] = 0;

        if(PIND & (1<<PC3))
        {
            if(!last[1])
            {
                uart_buf_puts("Tor 2\n");
                last[1] = 1;
            }
        }
        else
            last[1] = 0;

        if(last[0] || last[1])
            PORTC = 0x0;
        else
            PORTC = 0xff;

    }

/*
uint8_t chgd[5] = {0,0,0,0,0};
    uart_init(UBRR_VALUE);

    sei();

    while(1)
    {
        if(PINB & (1<<PC0))
        {
            if(!chgd[0])
            {
                uart_buf_puts("Taster 1 an\n");
                chgd[0] = 1;
            }
        }
        else
        {
            if(chgd[0])
            {
                uart_buf_puts("Taster 1 aus\n");
                chgd[0] = 0;
            }
        }

        if(PINB & (1<<PC1))
        {
            if(!chgd[1])
            {
                uart_buf_puts("Taster 2 an\n");
                chgd[1] = 1;
            }
        }
        else
        {
            if(chgd[1])
            {
                uart_buf_puts("Taster 2 aus\n");
                chgd[1] = 0;
            }
        }

        if(PINB & (1<<PC2))
        {
            if(!chgd[2])
            {
                uart_buf_puts("Taster 3 an\n");
                chgd[2] = 1;
            }
        }
        else
        {
            if(chgd[2])
            {
                uart_buf_puts("Taster 3 aus\n");
                chgd[2] = 0;
            }
        }


        if(PINB & (1<<PC3))
        {
            if(!chgd[3])
            {
                uart_buf_puts("Taster 4 an\n");
                chgd[3] = 1;
            }
        }
        else
        {
            if(chgd[3])
            {
                uart_buf_puts("Taster 4 aus\n");
                chgd[3] = 0;
            }
        }


        if(PINB & (1<<PC4))
        {
            if(!chgd[4])
            {
                uart_buf_puts("Taster 5 an\n");
                chgd[4] = 1;
            }
        }
        else
        {
            if(chgd[4])
            {
                uart_buf_puts("Taster 5 aus\n");
                chgd[4] = 0;
            }
        }

    }
*/

/*
DDRC = 0xff;
PORTC = 0x0;

 uart_init(UBRR_VALUE);
while(1)
{
_delay_ms(1000);
PORTC = ~PORTC;
uart_putc('x'); 
}

*/
/*

while(1)
    {
}
*/

    /* wird nie erreicht */
    return 0;
}
