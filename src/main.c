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

#define NUMBER_OF_MENU_ENTRIES 2

const char menu_entries[NUMBER_OF_MENU_ENTRIES][4] = { "Neu ", "Tore" };

static seven_seg sseg;
uint8_t tore[2];
uint8_t menu_idx;
button *active_buttons;
boolean change_value;

void flash_sseg(void)
{
    // Make atomic?
    sseg.inverted = FALSE;
    _delay_ms(10);
    sseg.inverted = TRUE;
}

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

void update_menu(void)
{
    char text[4];
    
    if(change_value)
    {

        switch(menu_idx)
        {
        case 0:
            /* Neu */
            strncpy(text, "Go  ", 4);
            break;
        case 1:
            /* Tore */
            strncpy(text, "T 42", 4);
            break;
        default:
            //???? WTF
            break;
        }
    }
    else
    {
        strncpy(text, menu_entries[menu_idx], 4);
    }
    seven_seg_set_chr(&sseg, text);
}

void chg_menu_idx(void *p)
{
    change_value = FALSE;
    menu_idx = menu_idx + ((int16_t) p) % NUMBER_OF_MENU_ENTRIES;
    update_menu();
}

void switch_to_menu(void *p)
{
    active_buttons = (button*) p;
    menu_idx = 0;
    change_value = FALSE;
    update_menu();
}

void switch_to_game(void *p)
{
    active_buttons = (button*) p;
    update_sseg();
}

void chg_max_goals(int8_t change)
{
    uint8_t tore_tmp = 10; // REMOVE IF SETTINGS STRUCT IS IMPLEMENTED
    int16_t new = tore_tmp + change;
    if (new > 0 && new <= 99)
    {
        tore_tmp = new;
    }
    else
    {
        flash_sseg();
    }
}

void chg_menu_value(void *p)
{
    int16_t chg = (int16_t) p;
    if(change_value)
    {
        switch(menu_idx)
        {
        case 0:
            /* Neu */
            break;
        case 1:
            /* Tore */
            chg_max_goals(chg);
            break;
        default:
            // WHAT TO FO HERE?
            break;
        }
        // change value here
    }
    else
    {
        change_value = TRUE;
    }
}

int main(void)
{
    const uint8_t delay= 2;

    shift_reg reg;
    /* game "buttons" */
    button butt;

    /* menu buttons */
    button menu;
    button_init(&menu);

    button_add(&menu, &PINB, PB0, chg_menu_idx, -1);
    button_add(&menu, &PINB, PB1, chg_menu_idx, +1);
    
    button_add(&menu, &PINB, PB2, switch_to_game, &butt);

    button_add(&menu, &PINB, PB3, chg_menu_value, -1);
    button_add(&menu, &PINB, PB4, chg_menu_value, +1);
    
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
    
    button_add(&butt, &PINB, PB2, switch_to_menu, &menu);
    
    button_add(&butt, &PINB, PB3, dector, 1);
    button_add(&butt, &PINB, PB4, inctor, 1);
    
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
