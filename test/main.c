#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>

#include <util/delay.h>

#include <stdint.h>

#define FOREVER for(;;)

int main(void)
{
    DDRC = 0xff;
    PORTC = 0xff;
    
    FOREVER
    {
        _delay_ms(1000);
        PORTC = ~PORTC;
    }

    /* wird nie erreicht */
    return 0;
}
