#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <avr/interrupt.h>
#include <avr/io.h>

#include <util/delay.h>
#include <util/atomic.h>

#define BAUD 19200
#include <util/setbaud.h>

#include "button.h"
#include "shift_reg.h"
#include "seven_seg.h"
#include "types.h"
#include "uart.h"

#define NUM_GOALS (2)
#define DISPLAY_ELEMENTS_PER_GOAL (2)

#define DEFAULT_GOALS_PER_ROUND (10)
#define DEFAULT_BEEPER (TRUE)
#define DEFAULT_BEEP_TIME (05)
#define DEFAULT_LOCK_TIME (20)

#define NUMBER_OF_MENU_ENTRIES (2)

const char menu_entries[NUMBER_OF_MENU_ENTRIES][4] = { "Neu ", "Tore", "LocT", "BepT" };

uint8_t menu_idx;
button *active_buttons;
boolean change_value;

struct settings_s
{
    uint8_t goals_per_round;
    boolean beeper;
    uint8_t beep_time;
    uint8_t lock_time;
};

static struct settings_s settings = {DEFAULT_GOALS_PER_ROUND, DEFAULT_BEEPER, DEFAULT_BEEP_TIME, DEFAULT_LOCK_TIME};

static seven_seg sseg;

static uint8_t goals[NUM_GOALS] = {0,0};

static volatile uint16_t beeper = 0;
static volatile uint16_t locked = 0;

void ntostr(char *result, uint8_t n)
{
    /* max val "255" + \0  = 4 chars*/
    char buf[4];
    
    itoa(n,buf,10);
    
    if(n<10)
    {
        result[0] = ' ';
        result[1] = buf[0];
    }
    else        
        strncpy(result,buf,2);
}

void flash_sseg(void)
{
    // Make atomic?
    sseg.inverted = !sseg.inverted;
    _delay_ms(10);
    sseg.inverted = !sseg.inverted;
}

void update_sseg(void)
{
    char stext[NUM_GOALS * DISPLAY_ELEMENTS_PER_GOAL];
    size_t n = 0;
    
    for(;n < NUM_GOALS; ++n)
    {
        ntostr(stext+n*DISPLAY_ELEMENTS_PER_GOAL, goals[n]);
    }
    
    seven_seg_set_chr(&sseg, stext);
}

void incgoal(void *p)
{
    if(goals[(size_t)p] < settings.goals_per_round)
    {
        goals[(size_t)p] += 1;
    }
    update_sseg();
}

void decgoal(void *p)
{
    if(goals[(size_t)p] > 0)
    {
        goals[(size_t)p] -= 1;
    }
    update_sseg();
}

void goal(void *p)
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        if(!locked)
        {
            incgoal(p);
            seven_seg_set_dot(&sseg, 0b1111);
        
            if(settings.beeper)
            {
                beeper = 500;
                PORTD |= (1<<PD4); /* beeper on */   
            }
            locked = 2000;
            
        }
    }
}

void self_test()
{
    uint16_t n = 0;
    
    uart_buf_puts("Hello!\n");
    
    seven_seg_set_chr(&sseg, "8888");
    seven_seg_set_dot(&sseg, 0b1111);
    PORTC &= ~(1<<PC7); /* err LED on */
    PORTD |= (1<<PD4); /* beeper on */
    
    for(;n < 250; ++n)
    {
        seven_seg_loop(&sseg);
        _delay_ms(2);
    }
    
    update_sseg();
    seven_seg_set_dot(&sseg, 0);
    PORTD &= ~(1<<PD4); /* beeper off */
    PORTC |= (1<<PC7); /* err LED off */
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
            PORTD &= ~(1 << PD4);
        }
    }
    if(locked)
    {
        --locked;
        if (!locked)
        {
            seven_seg_set_dot(&sseg, 0);
        }
    }
}

void update_menu(void)
{
    char text[4];
    uint8_t dots = 0;
    
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
            //TODO: Insert value
            strncpy(text, "T 42", 4);
            break;
        case 2:
            /* LocT */
            //TODO: Insert value
            strncpy(text, "LT42", 4);
            dots = 0b10;
            break;
        case 3:
            /* BepT */
            //TODO: Insert value
            strncpy(text, "BT42", 4);
            dots = 0b10;
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
    seven_seg_set_dot(&sseg, dots);
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

void chg_value(uint8_t *value, int8_t change, uint8_t lower, uint8_t upper)
{
    int16_t new = *value + change;
    if (new >= lower && new <= upper)
    {
        *value = new;
    }
    else
    {
        flash_sseg();
    }
}

void chg_time(uint8_t *time, int8_t change)
{
    chg_value(time, change, 0, 99);
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
            chg_value(&settings.goals_per_round, chg, 1, 99);
            break;
        case 2:
            /* LocT */
            chg_time(settings.lock_time, chg);
            break;
        case 3:
            /* BepT */
            chg_time(settings.beep_time, chg);
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
    const mc_pin seg_cat[] = {PC3, PC4, PC5, PC6};
    const uint8_t display_convert_table[] = {6, 3, 7, 4, 2, 1, 0, 5};

    shift_reg reg;
    size_t n = 0;

    /* game "buttons" */
    button butt;

    /* menu buttons */
    button menu;
    
    /* Init */
    uart_init(UBRR_VALUE);
    
    button_init(&butt);
    button_init(&menu);
    
    shift_reg_init(&reg, &PORTC, PC0, PC2, PC1);    
    seven_seg_init(&sseg, NUM_GOALS * DISPLAY_ELEMENTS_PER_GOAL, &PORTC, &reg, seg_cat, display_convert_table, TRUE);
    
    /* unused */
    DDRA = 0x0;
    PORTA = 0x0;
    
    /* ISP & buttons */
    DDRB = 0x0;
    PORTB = 0x0;
    
    /* err LED (PC7), shift reg, element driver */
    DDRC  = 0xff;
    PORTC = 0x0;
    
    /* beeper (PD4), unused driver pins (PD4-PD7), uart, light barriers (PD2,PD3) */
    DDRD = 0b11110000;
    PORTD = 0x0;
    
    /* self test */
    self_test();

    /* Configure game buttons */
    button_add(&butt, &PIND, PD2, goal, 0);
    button_add(&butt, &PIND, PD3, goal, 1);

    button_add(&butt, &PINB, PB0, decgoal, 0);
    button_add(&butt, &PINB, PB1, incgoal, 0);
    
    button_add(&butt, &PINB, PB2, switch_to_menu, &menu);
    
    button_add(&butt, &PINB, PB3, decgoal, 1);
    button_add(&butt, &PINB, PB4, incgoal, 1);

    /* Configure menu buttons */
    button_add(&menu, &PINB, PB0, chg_menu_idx, -1);
    button_add(&menu, &PINB, PB1, chg_menu_idx, +1);
    
    button_add(&menu, &PINB, PB2, switch_to_game, &butt);

    button_add(&menu, &PINB, PB3, chg_menu_value, -1);
    button_add(&menu, &PINB, PB4, chg_menu_value, +1);

    /* Configure timer 0 */
    TCCR0 |= (1 << WGM01); /* CTC mode */
    TCCR0 |= (1 << CS00) | (1 << CS01); /* Prescaler 64 */
    OCR0 = 249; /* ((16000000/64)/1000) = 250; decreased by one due of starting with zero */
    TIMSK |= (1<<OCIE0); /* activate timer */

    sei();
    
    while(1)
    {
        for(n = 0; n < 10; ++n)
        {
            button_poll(active_buttons);
        }
        seven_seg_loop(&sseg);
    }

    /* wird nie erreicht */
    return 0;
}
