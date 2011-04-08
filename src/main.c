#include <math.h>

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>

#include <avr/delay.h>

#define BAUD 19200
#include <util/setbaud.h>

#include "shift_reg.h"
#include "seven_seg.h"
#include "uart.h"
#include "types.h"

static seven_seg sseg;

int main(void)
{
const uint8_t delay= 500;

    shift_reg reg;

//TODO: Use correct C syntax
    char play[] = "Play";
    uint8_t n = 0;
    
    DDRC  = 0xff;
    PORTC = 0x0;
    
    DDRD = 0xff;
    PORTD = 0x00;

    shift_reg_init(&reg, &PORTC, PC0, PC2, PC1);
    uart_init(UBRR_VALUE);

    //PORTD |= (1 << PD4);

    //while(1) {PORTD = ~PORTD; _delay_ms(1000);}
    
 
    ///PORTC &= ~(1 << PC7); //LED an
    // PORTC |= (1 << PC7);
    
    //shift_reg_write(&reg, 0/*0b11101111*/);
    
    while(1);
    while(1) {
    for(n = 0; n<5; ++n)
    {
    PORTC |= (1 << PC3);
    _delay_ms(delay);
    PORTC &= ~(1 << PC3);
    PORTC |= (1 << PC4);
    _delay_ms(delay);
    PORTC &= ~(1 << PC4);
    PORTC |= (1 << PC5);
    _delay_ms(delay);
    PORTC &= ~(1 << PC5);
    PORTC |= (1 << PC6);
    _delay_ms(delay);
    PORTC &= ~(1 << PC6);
    uart_puts("foo");
    }
    }
//    PORTC |= (1 << PC6);
    ///*
    mc_pin seg_cat[] = { PC3, PC4, PC5, PC6 };
    uint8_t table[] = {0, 1, 2, 3, 4, 5, 6, 7};
    
    seven_seg_init(&sseg, 4, &PORTC, &reg, seg_cat, table, 1);

    uint8_t vals[] = {0b11110111, 0b11111011, 0b11111101, 0b11111110 };

    seven_seg_set_val(&sseg, vals);
//    seven_seg_set_chr(&sseg, play);
    sei();
    while(1)
    {
        seven_seg_loop(&sseg);
        _delay_ms(delay);
    }
//*/
    /* wird nie erreicht */
    return 0;
}

uint8_t* generateTable(seven_seg *sseg)
{
    uint8_t table[] = {0, 1, 2, 3, 4, 5, 6, 7};
    uint8_t test[] = {0, 0, 0, 0};
    
    uint8_t testDisplay = 0;
    uint8_t testSegment;


    for (uint8_t i = 0; i < 4; i++)
    {
    
    
        //TODO: As long as the "define next segment" button is not pushed
        while (1)
        {
            //TODO: As long as the "test with next segment" button is pushed
            testSegment++;
            test[testDisplay] = 1 << (testSegment % 8);
        }
    }
    
    
    
    
}
