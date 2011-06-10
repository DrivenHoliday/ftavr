#ifndef SETTINGS_H
#define SETTINGS_H

#include "types.h"

#define DEFAULT_GOALS_PER_ROUND (10)
#define MIN_GOALS_PER_ROUND     (01)
#define MAX_GOALS_PER_ROUND     (99)

#define DEFAULT_BEEPER          (TRUE)

#define DEFAULT_BEEP_TIME       (05)
#define MIN_BEEP_TIME           (01)
#define MAX_BEEP_TIME           (99)

#define DEFAULT_LOCK_TIME       (20)
#define MIN_LOCK_TIME           (00)
#define MAX_LOCK_TIME           (99)

#define DEFAULT_ERROR_TIME      (03)
#define MIN_ERROR_TIME          (00)
#define MAX_ERROR_TIME          (99)

#define DEFAULT_BOUNCER_TIME    (10)
#define MIN_BOUNCER_TIME        (00)
#define MAX_BOUNCER_TIME        (99)

struct settings_s
{
    uint8_t goals_per_round;
    
    /* Defines if beeper is activated (overrides beep_time) */
    boolean beeper;
    
    /* time in 100 ms steps */
    uint8_t beep_time;
    
    /* time in 100 ms steps */
    uint8_t lock_time;
    
    /* time in 100 ms steps */
    uint8_t error_time;
    
    /* time in 10 ms steps */
    uint8_t bouncer_time;
};

struct settings_s* settings(void);

void settings_read(void);
void settings_write(void);
void settings_reset(void);

#endif
