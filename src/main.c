#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/eeprom.h>
#include <avr/wdt.h>

#include <util/delay.h>
#include <util/atomic.h>

#define BAUD 19200
#include <util/setbaud.h>

#include "button.h"
#include "config.h"
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
#define DEFAULT_BOUNCER_TIME (01)

static button game_buttons;
static button menu_buttons;
static button *active_buttons = NULL;
static menu_entries entries;

/* Settings eeprom pointer */
uint8_t ee_goals_per_round EEMEM = DEFAULT_GOALS_PER_ROUND;
boolean ee_beeper EEMEM = DEFAULT_BEEPER;
uint8_t ee_beep_time EEMEM = DEFAULT_BEEP_TIME;
uint8_t ee_lock_time EEMEM = DEFAULT_LOCK_TIME;
uint8_t ee_error_time EEMEM = DEFAULT_ERROR_TIME;
uint8_t ee_bouncer_time EEMEM = DEFAULT_BOUNCER_TIME;

struct settings_s
{
    uint8_t goals_per_round;
    // Defines if beeper is activated (overrides beep_time)
    boolean beeper;
    // time in 100 ms steps
    uint8_t beep_time;
    // time in 100 ms steps
    uint8_t lock_time;
    // time in 100 ms steps
    uint8_t error_time;
    // time in 10 ms steps
    uint8_t bouncer_time;
};

static struct settings_s settings = {DEFAULT_GOALS_PER_ROUND, DEFAULT_BEEPER, DEFAULT_BEEP_TIME, DEFAULT_LOCK_TIME, DEFAULT_ERROR_TIME, DEFAULT_BOUNCER_TIME};

static seven_seg sseg;

static uint8_t goals[NUM_GOALS] = {0,0};
static volatile uint8_t last_goal = 255;

static volatile uint16_t beeper = 0;
static volatile uint16_t locked = 0;
static volatile uint16_t error = 0;
static volatile uint16_t bouncer = 0;

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

void beeper_on(void)
{
#if CONF_BEEPER_ON == LOW
    PORTD &= ~(1<<CONF_BEEPER_PIN);
#else
    PORTD |= (1<<CONF_BEEPER_PIN);
#endif
}

void beeper_off(void)
{
#if CONF_BEEPER_ON == LOW
   PORTD |= (1<<CONF_BEEPER_PIN);
#else
    PORTD &= ~(1<<CONF_BEEPER_PIN);
#endif
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
            beeper_on();
        }       
    }
}

void sseg_set_last_goal_dots(void)
{
    if(active_buttons == &game_buttons)
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
        ++goals[(size_t)p];
        sseg_display_goals();
    }
}

void decgoal(void *p)
{
    if(goals[(size_t)p] > 0)
    {
        --goals[(size_t)p];
        sseg_display_goals();
    }
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
            beeper_off();
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
                beeper_off();
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

    if(bouncer)
    {
        --bouncer;
        if (!bouncer)
        {
            count_goal(last_goal);
        }
    }

    ++ud;
}

void count_goal(size_t goal)
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        incgoal(goal);
        if(settings.beeper && (settings.beep_time > 0))
        {
            beeper = ((uint16_t) settings.beep_time) * 100;
            beeper_on();
        }
        if (settings.lock_time > 0)
        {
            seven_seg_set_dot(&sseg, 0b1111);
            locked = ((uint16_t) settings.lock_time) * 100;
        }
    }
}

void goal(void *p)
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        if(bouncer)
        {
            /* bouncer detected */
            bouncer = 0;
        }
        else if(!locked)
        {
            last_goal = p;
            bouncer = ((uint16_t) settings.bouncer_time) * 10;

            if(!bouncer)
            {
                count_goal(p);
                beeper = ((uint16_t) settings.beep_time) * 100;
                beeper_on();
            }
        }
    }
}

void self_test()
{    
    uart_buf_puts("Hello!\n");
    
    seven_seg_set_chr(&sseg, "8888");
    seven_seg_set_dot(&sseg, 0b1111);
    PORTC &= ~(1<<CONF_LED_PIN); /* err LED on */
    beeper_on();

    loop_display(500);
    
    sseg_display_goals();
    seven_seg_set_dot(&sseg, 0);
    beeper_off();
    PORTC |= (1<<CONF_LED_PIN); /* err LED off */
}

void chg_menu_idx(void *p)
{
    menu_entry_chg_select(&entries, (int8_t) p);
    menu_entry_display(&entries, &sseg);
}

void switch_buttons(button *succ)
{
    button_poll_action(succ, FALSE);
    active_buttons = succ;
}

void switch_to_menu(void *p)
{
    switch_buttons(&menu_buttons);
    menu_entry_display_entry(&entries, &sseg, 0);
}

void write_settings(void)
{
    eeprom_write_byte(&ee_goals_per_round, settings.goals_per_round);
    eeprom_write_byte(&ee_beep_time, settings.beep_time);
    eeprom_write_byte(&ee_beeper, settings.beeper);
    eeprom_write_byte(&ee_lock_time, settings.lock_time);
    eeprom_write_byte(&ee_error_time, settings.error_time);
    eeprom_write_byte(&ee_bouncer_time, settings.bouncer_time);
}

void switch_to_game(void *p)
{
    switch_buttons(&game_buttons);
    write_settings();
    sseg_display_goals();
}

void menu_entry_value_bool_toogle(void *p)
{
    *((boolean*) p) = !(*((boolean*) p));
    menu_entry_display(&entries, &sseg);
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

void sos(void)
{
    const uint16_t short_s = 120;
    const uint16_t long_s = short_s * 3;
    
    const uint16_t pause_s = short_s;     /* symbole */
    const uint16_t pause_c = long_s - pause_s;      /* character */
    const uint16_t pause_w = (short_s * 7) - pause_s; /* word */
    
    size_t n = 0;
    
    for(n = 0; n < 3; ++n)
    {
        beeper_on();
        _delay_ms(short_s);
        beeper_off();
        _delay_ms(pause_s);
    }
    
    _delay_ms(pause_c);
    
    
    for(n = 0; n < 3; ++n)
    {
        beeper_on();
        _delay_ms(long_s);
        beeper_off();
        _delay_ms(pause_s);
    }
     
    _delay_ms(pause_c);
    
    for(n = 0; n < 3; ++n)
    {
        beeper_on();
        _delay_ms(short_s);
        beeper_off();
        _delay_ms(pause_s);
    }
    
    _delay_ms(pause_w);
}

void watch_dog(void)
{
    /* if watch dog reset */
    if(MCUCSR & (1<<WDRF))
    {
        /* delete watch dog reset flag */
        MCUCSR &= ~(1<<WDRF);
        
        /* communicate error state */
        PORTC &= ~(1<<CONF_LED_PIN); /* err LED on */
       
        while(1)
        {
            sos();
        }
    }
    
    wdt_enable(WDTO_1S);
}

void menu_entry_reset(void *p)
{
    settings.goals_per_round = DEFAULT_GOALS_PER_ROUND;
    settings.beeper = DEFAULT_BEEPER;
    settings.beep_time = DEFAULT_BEEP_TIME;
    settings.lock_time = DEFAULT_LOCK_TIME;
    settings.error_time = DEFAULT_ERROR_TIME;
    settings.bouncer_time = DEFAULT_BOUNCER_TIME;
//    write_settings();    
}

void menu_entry_start(void *p)
{

    /*******************/
    //TODO: START GAME //
    /*******************/

    last_goal = 255;
    goals[0] = 0;
    goals[1] = 0;

    switch_to_game(NULL);
}

int main(void)
{
    const mc_pin seg_cat[] = CONF_DISPLAY_ELEMENT_PINS;
    const uint8_t display_convert_table[] = CONF_DISPLAY_CONVERT_TABLE;

    shift_reg reg;

    /* load settings */
    settings.goals_per_round = eeprom_read_byte(&ee_goals_per_round);
    settings.beeper = eeprom_read_byte(&ee_beeper);
    settings.lock_time = eeprom_read_byte(&ee_lock_time);
    settings.beep_time = eeprom_read_byte(&ee_beep_time);
    settings.error_time = eeprom_read_byte(&ee_error_time);
    
     /* unused */
    DDRA  = 0x0;
    PORTA = 0x0;
    
    /* ISP & buttons */
    DDRB  = 0x0;
    PORTB = 0x0;
    
    /* err LED, shift reg, element driver */
    DDRC  = 0xff;
    PORTC = 0x0;
    
    /* beeper (PD4), unused driver pins (PD5-PD7), uart (PD0, PD1), light barriers (PD2, PD3) */
    DDRD  = 0b11110000;
#if CONF_GOAL_PULL_UPS_ENABLED == TRUE
    PORTD = 0b00001100;
#else
    PORTD = 0x0;
#endif
   
    /* Watch Dog */
    watch_dog();
       
    /* Init */
    uart_init(UBRR_VALUE);
    
    button_init(&game_buttons);
    button_init(&menu_buttons);
    
    shift_reg_init(&reg, &CONF_SHIFT_REG_PORT, CONF_SHIFT_REG_SER_PIN, CONF_SHIFT_REG_SCK_PIN, CONF_SHIFT_REG_RCK_PIN);    
    seven_seg_init(&sseg, NUM_DISPLAY_ELEMENTS, &PORTC, &reg, seg_cat, display_convert_table, CONF_SSEG_INVERT);
    
    /* self test */
    self_test();

    /* Configure game buttons */
    button_add(&game_buttons, &PIND, CONF_GOAL_PIN_0, goal, 0);
    button_add(&game_buttons, &PIND, CONF_GOAL_PIN_1, goal, 1);

    button_add(&game_buttons, &PINB, CONF_BUTTON_PIN_0, decgoal, 0);
    button_add(&game_buttons, &PINB, CONF_BUTTON_PIN_1, incgoal, 0);
    
    button_add(&game_buttons, &PINB, CONF_BUTTON_PIN_2, switch_to_menu, NULL);
    
    button_add(&game_buttons, &PINB, CONF_BUTTON_PIN_3, decgoal, 1);
    button_add(&game_buttons, &PINB, CONF_BUTTON_PIN_4, incgoal, 1);

    /* Configure menu buttons */
    button_add(&menu_buttons, &PINB, CONF_BUTTON_PIN_0, chg_menu_idx, -1);
    button_add(&menu_buttons, &PINB, CONF_BUTTON_PIN_1, chg_menu_idx, +1);
    
    button_add(&menu_buttons, &PINB, CONF_BUTTON_PIN_2, switch_to_game, NULL);

    /* Add menu entries */
    menu_entry_init(&entries, menu_error, &sseg);

    menu_entry_button_left(&entries, &menu_buttons, &PINB, CONF_BUTTON_PIN_3);
    menu_entry_button_right(&entries, &menu_buttons, &PINB, CONF_BUTTON_PIN_4);

    // Start a new game
    menu_entry_add(&entries, "Go", menu_entry_get_go, menu_entry_start, menu_entry_start, NULL);
    // Goals limit
    menu_entry_add_int(&entries, "GL", 1, 99, 0b00, &settings.goals_per_round);
    // Lock time
    menu_entry_add_int(&entries, "Lt", 0, 99, 0b10, &settings.lock_time);
    // Bouncer time
    menu_entry_add_int(&entries, "bc", 0, 99, 0b00, &settings.bouncer_time);
    // Beeper activated (quick on/off)
    menu_entry_add(&entries, "Ba", menu_entry_get_active, menu_entry_value_bool_toogle, menu_entry_value_bool_toogle, &(settings.beeper));
    // Beeper time
    menu_entry_add_int(&entries, "Bt", 0, 99, 0b10, &settings.beep_time);
    // Error time
    menu_entry_add_int(&entries, "Et", 0, 99, 0b10, &settings.error_time);
    // Resets to default
    menu_entry_add(&entries, "Re", menu_entry_get_go, menu_entry_reset, menu_entry_reset, NULL);
    
    /* Set by default the menu to active. */
    switch_to_menu(NULL);

    /* Configure timer 0 */
    TCCR0 |= (1 << WGM01); /* CTC mode */
    TCCR0 |= (1 << CS00) | (1 << CS01); /* Prescaler 64 */
    OCR0 = 249; /* ((16000000/64)/1000) = 250; decreased by one due of starting with zero */
    TIMSK |= (1<<OCIE0); /* activate timer */

    sei();
    
    while(1)
    {
        wdt_reset();
        
        button_poll(active_buttons);

        if(ud > 3)
        {
            seven_seg_loop(&sseg);
            ud = 0;
        }
    }

    /* wird nie erreicht */
    return 0;
}
