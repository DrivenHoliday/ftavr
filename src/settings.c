#include "settings.h"

#include <avr/eeprom.h>

struct settings_s ee_settings EEMEM;

struct settings_s* settings(void)
{
    static struct settings_s settings;
    return &settings;
}

#define BETWEEN(var,min,max) \
    ((var >= min) && (var <= max))

static boolean settings_valid()
{
    if( BETWEEN(settings()->goals_per_round, MIN_GOALS_PER_ROUND, MAX_GOALS_PER_ROUND) &&
        BETWEEN(settings()->beep_time,       MIN_BEEP_TIME,       MAX_BEEP_TIME) &&
        BETWEEN(settings()->lock_time,       MIN_LOCK_TIME,       MAX_LOCK_TIME) &&
        BETWEEN(settings()->bouncer_time,    MIN_BOUNCER_TIME,    MAX_BOUNCER_TIME) )
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

void settings_read(void)
{
    eeprom_read_block(settings(), &ee_settings, sizeof(ee_settings));
    
    if(!settings_valid())
    {
        settings_reset();
    }
}

void settings_write(void)
{
    eeprom_write_block(settings(), &ee_settings, sizeof(ee_settings));
}

void settings_reset(void)
{
    settings()->goals_per_round = DEFAULT_GOALS_PER_ROUND;
    settings()->beeper          = DEFAULT_BEEPER;
    settings()->beep_time       = DEFAULT_BEEP_TIME;
    settings()->lock_time       = DEFAULT_LOCK_TIME;
    settings()->bouncer_time    = DEFAULT_BOUNCER_TIME;
    
    settings_write();
}
