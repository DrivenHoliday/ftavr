#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>

#include <avr/delay.h>

#include <stdint.h>

#define BAUD 19200
#include <util/setbaud.h>

#include "uart.h"
#include "shift_reg.h"
#include "seven_seg.h"
#include "button.h"

#define FOREVER for(;;)

void flash(void *p)
{
    PORTC &= ~(1<<PC5);
    PORTD &= ~(1<<PD4);
    _delay_ms(500); 
    PORTC |= (1<<PC5);
    PORTD |= (1<<PD4);
}

int main(void)
{
    const mc_pin seg_cat[] = {PC0, PC1, PC2, PC3, PC4};
    const uint8_t display_convert_table[] = {6,5,3,1,2,0,7,4};
    
    uint8_t n = 0;
    
    shift_reg reg;
    seven_seg sseg;
    button butt;
    
    DDRD  = 0b11110000;
    PORTD = 0b00001100; //pullup 1
    
    DDRC = 0xff;
    PORTC = 0xff;
    
    DDRB = 0x0;
    PORTB = 0x0;

    uart_init(UBRR_VALUE);
    
    uart_puts("init\n");
    
    shift_reg_init(&reg, &PORTD, PD7, PD5, PD6);
    
    seven_seg_init(&sseg, 5, &PORTC, &reg, seg_cat, display_convert_table, TRUE);
    
    seven_seg_set_chr(&sseg, "gut\0\0");
    seven_seg_set_dot(&sseg, 0b10101);
    
    button_init(&butt);
    
    button_add(&butt, &PIND, PD2, flash, 0);
    button_add(&butt, &PIND, PD3, flash, 1);

    button_add(&butt, &PINB, PB0, flash, 0);
    button_add(&butt, &PINB, PB1, flash, 0);
    button_add(&butt, &PINB, PB2, flash, 1);
    button_add(&butt, &PINB, PB3, flash, 1);
    button_add(&butt, &PINB, PB4, flash, 1);
    
    flash(NULL);
    
    sei();
    
    FOREVER
    {
        seven_seg_loop(&sseg);
        button_poll(&butt);
        
        _delay_ms(1); 
    }

    /* wird nie erreicht */
    return 0;
}
