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
#include "menu_entry.h"

#define NUM_GOALS (2)
#define DISPLAY_ELEMENTS_PER_GOAL (2)
#define NUM_DISPLAY_ELEMENTS (NUM_GOALS*DISPLAY_ELEMENTS_PER_GOAL)

#define DEFAULT_GOALS_PER_ROUND (10)
#define DEFAULT_BEEPER (TRUE)
#define DEFAULT_BEEP_TIME (05)
#define DEFAULT_LOCK_TIME (20)
#define DEFAULT_ERROR_TIME (03)

static button *active_buttons = NULL;
static menu_entries entries;

struct settings_s
{
    uint8_t goals_per_round;
    boolean beeper;
    uint8_t beep_time;
    uint8_t lock_time;
    uint8_t error_time;
};

static struct settings_s settings = {DEFAULT_GOALS_PER_ROUND, DEFAULT_BEEPER, DEFAULT_BEEP_TIME, DEFAULT_LOCK_TIME, DEFAULT_ERROR_TIME};

static seven_seg sseg;

static uint8_t goals[NUM_GOALS] = {0,0};
static volatile uint8_t last_goal = 255;

static volatile uint16_t beeper = 0;
static volatile uint16_t locked = 0;
static volatile uint16_t error = 0;

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

/**
 * Zeigt den aktuellen Wert für ms millisekunden an.
 * Die Zeit in millisekunden muss gerade sein.
 */
void loop_display(uint16_t ms)
{
    while (ms)
    {
        seven_seg_loop(&sseg);
        _delay_ms(1);
        --ms;
    }
}

void flash_sseg(void)
{
    // Make atomic?
    sseg.inverted = ~sseg.inverted;
    loop_display(100);
    sseg.inverted = ~sseg.inverted;
}

void menu_error(void)
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        if(error > 0)
        {
            sseg.inverted = !sseg.inverted;
        }
        error = settings.error_time * 100;
        if(error > 0)
        { 
            sseg.inverted = !sseg.inverted;
            PORTD |= (1<<PD4); /* beeper on */
        }       
    }
}

void sseg_set_last_goal_dots(void)
{
    uint8_t dots = 0;
    size_t n = 0;
    
    for(n = 0; n < NUM_GOALS; ++n)
    {
        dots <<= 2;
        if(last_goal == n)
        {    
            dots |= 0b00000010;
        }
    }
    
    uart_buf_puti8(dots);
    
    seven_seg_set_dot(&sseg, dots);
}

void sseg_display_goals(void)
{
    char stext[NUM_DISPLAY_ELEMENTS];
    size_t n = 0;
    
    for(n = 0; n < NUM_GOALS; ++n)
    {
        ntostr(stext+n*DISPLAY_ELEMENTS_PER_GOAL, goals[n]);
    }
    
    seven_seg_set_chr(&sseg, stext);
    
    if(!locked)
    {
        sseg_set_last_goal_dots();
    }
}

void incgoal(void *p)
{
    if(goals[(size_t)p] < settings.goals_per_round)
    {
        goals[(size_t)p] += 1;
    }
    sseg_display_goals();
}

void decgoal(void *p)
{
    if(goals[(size_t)p] > 0)
    {
        goals[(size_t)p] -= 1;
    }
    sseg_display_goals();
}

static volatile uint8_t ud = 0;

/* Timer 0 interrupt vector, called every 1 ms */
ISR (TIMER0_COMP_vect)
{    
    if(beeper)
    {
        --beeper;
        if(!beeper && !error)
        {
            PORTD &= ~(1 << PD4);
        }
    }
    if(error)
    {
        --error;
        if(!error)
        {
            sseg.inverted = !sseg.inverted;
            if (!beeper)
            {
                PORTD &= ~(1 << PD4);
            }
        }
    }
    if(locked)
    {
        --locked;
        if (!locked)
        {
            sseg_set_last_goal_dots();
        }
    }
    ++ud;
}

void goal(void *p)
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        if(!locked)
        {
            incgoal(p);
            
            last_goal = p;
            
            seven_seg_set_dot(&sseg, 0b1111);
        
            if(settings.beeper && (settings.beep_time > 0))
            {
                beeper = ((uint64_t) settings.beep_time) * 100;
                PORTD |= (1<<PD4); /* beeper on */
            }
            locked = ((uint64_t) settings.lock_time) * 100;
            
        }
    }
}

void self_test()
{    
    uart_buf_puts("Hello!\n");
    
    seven_seg_set_chr(&sseg, "8888");
    seven_seg_set_dot(&sseg, 0b1111);
    PORTC &= ~(1<<PC7); /* err LED on */
    PORTD |= (1<<PD4); /* beeper on */

    loop_display(500);
    
    sseg_display_goals();
    seven_seg_set_dot(&sseg, 0);
    PORTD &= ~(1<<PD4); /* beeper off */
    PORTC |= (1<<PC7); /* err LED off */
}

void chg_menu_idx(void *p)
{
    menu_entry_chg_select(&entries, (int8_t) p);
    menu_entry_display(&entries, &sseg);
}

void switch_to_menu(void *p)
{
    active_buttons = (button*) p;
    menu_entry_display(&entries, &sseg);
}

void switch_to_game(void *p)
{
    active_buttons = (button*) p;
    sseg_display_goals();
}

void chg_value(uint8_t *value, int8_t change, uint8_t lower, uint8_t upper)
{
    int16_t new = ((int16_t) *value) + change;
    if (new >= lower && new <= upper)
    {
        *value = new;
    }
    else
    {
        menu_error();
    }
}

void menu_entry_value_gl_dec(void *p)
{
    chg_value((uint8_t*) p, -1, 1, 99);
}

void menu_entry_value_gl_inc(void *p)
{
    chg_value((uint8_t*) p, +1, 1, 99);
}

void menu_entry_value_dec(void *p)
{
    chg_value((uint8_t*) p, -1, 0, 99);
}

void menu_entry_value_inc(void *p)
{
    chg_value((uint8_t*) p, +1, 0, 99);
}

void menu_entry_value_bool_toogle(void *p)
{
    *((boolean*) p) = !(*((boolean*) p));
}

void menu_entry_get_value(void *p, char *ch, uint8_t *dots)
{
    ntostr(ch, *((uint8_t*) p));
    (*dots) = 0;
}

void menu_entry_get_value_dot(void *p, char *ch, uint8_t *dots)
{
    menu_entry_get_value(p, ch, dots);
    (*dots) = 0b0010;
}

void menu_entry_get_go(void *p, char *ch, uint8_t *dots)
{
    strncpy(ch, " ?", 2);
    (*dots) = 0b0010;
}

void menu_entry_get_active(void *p, char *ch, uint8_t *dots)
{
    if(*((boolean*) p))
    {
        strncpy(ch, "on", 2);
    }
    else
    {
        strncpy(ch, "of", 2);
    }
}

void menu_entry_start(void *p)
{

    /*******************/
    //TODO: START GAME //
    /*******************/

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
    seven_seg_init(&sseg, NUM_DISPLAY_ELEMENTS, &PORTC, &reg, seg_cat, display_convert_table, TRUE);
    
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

    /* Add menu entries */
    menu_entry_init(&entries, &sseg);

    menu_entry_button_left(&entries, &menu, &PINB, PB3);
    menu_entry_button_right(&entries, &menu, &PINB, PB4);

    // Start a new game
    menu_entry_add(&entries, "Go", menu_entry_get_go, menu_entry_start, menu_entry_start, NULL);
    // Goals limit
    menu_entry_add(&entries, "GL", menu_entry_get_value, menu_entry_value_gl_dec, menu_entry_value_gl_inc, &(settings.goals_per_round));
    // Lock time
    menu_entry_add(&entries, "Lt", menu_entry_get_value_dot, menu_entry_value_dec, menu_entry_value_inc, &(settings.lock_time));
    // Beeper activated (quick on/off)
    menu_entry_add(&entries, "Ba", menu_entry_get_active, menu_entry_value_bool_toogle, menu_entry_value_bool_toogle, &(settings.beeper));
    // Beeper time
    menu_entry_add(&entries, "Bt", menu_entry_get_value_dot, menu_entry_value_dec, menu_entry_value_inc, &(settings.beep_time));
    // Error time
    menu_entry_add(&entries, "Et", menu_entry_get_value_dot, menu_entry_value_dec, menu_entry_value_inc, &(settings.error_time));
    
    /* Set by default the menu to active. */
    switch_to_menu(&menu);

    /* Configure timer 0 */
    TCCR0 |= (1 << WGM01); /* CTC mode */
    TCCR0 |= (1 << CS00) | (1 << CS01); /* Prescaler 64 */
    OCR0 = 249; /* ((16000000/64)/1000) = 250; decreased by one due of starting with zero */
    TIMSK |= (1<<OCIE0); /* activate timer */

    sei();
    
    while(1)
    {
        //for(n = 0; n < 300; ++n)
        {
            button_poll(active_buttons);
        }
        if(ud > 3)
        {
            seven_seg_loop(&sseg);
            ud = 0;
        }
    }

    /* wird nie erreicht */
    return 0;
}
