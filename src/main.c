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
#define NUM_DISPLAY_ELEMENTS (NUM_GOALS*DISPLAY_ELEMENTS_PER_GOAL)

#define DEFAULT_GOALS_PER_ROUND (99)
#define DEFAULT_BEEPER (TRUE)

struct settings_s
{
    uint8_t goals_per_round;
    boolean beeper;
};

static struct settings_s settings = {DEFAULT_GOALS_PER_ROUND, DEFAULT_BEEPER};

static seven_seg sseg;

static uint8_t goals[NUM_GOALS] = {0,0};
static volatile uint8_t last_goal = 255;

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

/* Timer 0 interrupt vector, called every 1 ms */
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
            sseg_set_last_goal_dots();
        }
    }
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
    
    for(n = 0; n < 250; ++n)
    {
        seven_seg_loop(&sseg);
        _delay_ms(2);
    }
    
    sseg_display_goals();
    seven_seg_set_dot(&sseg, 0);
    PORTD &= ~(1<<PD4); /* beeper off */
    PORTC |= (1<<PC7); /* err LED off */
}

int main(void)
{
    const mc_pin seg_cat[] = {PC3, PC4, PC5, PC6};
    const uint8_t display_convert_table[] = {6, 3, 7, 4, 2, 1, 0, 5};

    shift_reg reg;
    button butt;
    size_t n = 0;
    
    /* Init */
    uart_init(UBRR_VALUE);
    
    button_init(&butt);
    
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
    
    /* Configure buttons */
    button_add(&butt, &PIND, PD2, goal, 0);
    button_add(&butt, &PIND, PD3, goal, 1);

    button_add(&butt, &PINB, PB0, decgoal, 0);
    button_add(&butt, &PINB, PB1, incgoal, 0);
    
    button_add(&butt, &PINB, PB3, decgoal, 1);
    button_add(&butt, &PINB, PB4, incgoal, 1);

    /* Configure timer 0 */
    TCCR0 |= (1 << WGM01); /* CTC mode */
    TCCR0 |= (1 << CS00) | (1 << CS01); /* Prescaler 64 */
    OCR0 = 249; /* ((16000000/64)/1000) = 250 */
    TIMSK |= (1<<OCIE0); /* activate timer */

    sei();
    
    while(1)
    {
        for(n = 0; n < 10; ++n)
        {
            button_poll(&butt);
            
            char c;
            if(uart_getc(&c))
            {
                uart_buf_putc(c);
            }
        }
        seven_seg_loop(&sseg);
    }

    /* wird nie erreicht */
    return 0;
}
