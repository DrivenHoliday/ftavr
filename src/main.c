#include <avr/interrupt.h>
#include <avr/io.h>

#include <util/delay.h>

#define BAUD 19200
#include <util/setbaud.h>

#include "shift_reg.h"
#include "seven_seg.h"
#include "types.h"
#include "uart.h"

static seven_seg sseg;

int main(void)
{
    const uint8_t delay= 2;

    shift_reg reg;
    
    const mc_pin seg_cat[] = {PC3, PC4, PC5, PC6};
    const uint8_t table[] = {6, 3, 7, 4, 2, 1, 0, 5};
    
    DDRC  = 0xff;
    PORTC = 0x0;
    
    DDRD = 0xff;
    PORTD = 0x00;

    uart_init(UBRR_VALUE);
    
    shift_reg_init(&reg, &PORTC, PC0, PC2, PC1);    
    seven_seg_init(&sseg, 4, &PORTC, &reg, seg_cat, table, TRUE);
    
    seven_seg_set_chr(&sseg, "1EE7");
    
    seven_seg_set_dot(&sseg, 0b1010);

    sei();
    
    while(1)
    {
        seven_seg_loop(&sseg);
        _delay_ms(delay);
    }

    /* wird nie erreicht */
    return 0;
}
