#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <avr/interrupt.h>
#include <avr/io.h>

#include <util/delay.h>

#define BAUD 19200
#include <util/setbaud.h>

#include "button.h"
#include "shift_reg.h"
#include "seven_seg.h"
#include "types.h"
#include "uart.h"

static seven_seg sseg;
uint8_t tore[2];
uint16_t beeper;
uint16_t locked;

void update_sseg(void)
{
    static char stext[4];
    static char buf[4];
    
    itoa(tore[0],buf,10);
    strncpy(stext,buf,2);
    
    itoa(tore[1],buf,10);
    strncpy(stext+2,buf,2);
    
    seven_seg_set_chr(&sseg, stext);
}

void inctor(void *p)
{
    tore[(size_t)p] = (tore[(size_t)p] + 1) % 100;
    update_sseg();
}

void dector(void *p)
{
    if(tore[(size_t)p] > 0) --tore[(size_t)p];
    update_sseg();
}

/*
 * Der Compare Interrupt Handler wird aufgerufen, wenn
 * TCNT0 = OCR0 = 250 - 1 
 * ist (250 Schritte), d.h. genau alle 1 ms
 */
ISR (TIMER0_COMP_vect)
{
    if(beeper)
    {
        --beeper;
        if(!beeper)
        {
            PORTD = ~(1 << PD4);
        }
    }
    if(locked)
    {
        --locked;
        if (!locked)
        {
            // Töre schörf mächen
        }
    }
}

int main(void)
{
    const uint8_t delay= 2;

    shift_reg reg;
    button butt;
    
    const mc_pin seg_cat[] = {PC3, PC4, PC5, PC6};
    const uint8_t table[] = {6, 3, 7, 4, 2, 1, 0, 5};
    
    DDRC  = 0xff;
    PORTC = 0x0;
    
    DDRD = 0b11110000;
    PORTD = 0x0;
    
    DDRB = 0x0;
    PORTB = 0x0;
    
    memset(tore, 0, sizeof(tore));

    uart_init(UBRR_VALUE);
    uart_buf_puts("Hallo!\n");
    
    shift_reg_init(&reg, &PORTC, PC0, PC2, PC1);    
    seven_seg_init(&sseg, 4, &PORTC, &reg, seg_cat, table, TRUE);
    
    seven_seg_set_chr(&sseg, "8888");
    
    seven_seg_set_dot(&sseg, 0b0000);
    
    button_init(&butt);
    
    button_add(&butt, &PIND, PD2, inctor, 0);
    button_add(&butt, &PIND, PD3, inctor, 1);

    button_add(&butt, &PINB, PB0, dector, 0);
    button_add(&butt, &PINB, PB1, inctor, 0);
    
    button_add(&butt, &PINB, PB3, dector, 1);
    button_add(&butt, &PINB, PB4, inctor, 1);

    // Configure timer 0
    TCCR0 |= (1 << WGM01); // CTC mode
    TCCR0 |= (1 << CS01) & (1 << CS00); // Prescaler 8
    // ((16000000/64)/1000) = 250
    OCR0 = 250 - 1;

    sei();
    
    while(1)
    {
        seven_seg_loop(&sseg);
        button_poll(&butt);
        _delay_ms(delay);
    }

    /* wird nie erreicht */
    return 0;
}
