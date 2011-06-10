#ifndef CONFIG_H
#define CONFIG_H

#include "types.h"

#define BROCKEL    (1)
#define BSCHWARTAU (2)
#define LOW        (1)
#define HIGH       (2)

#define CONF_CURRENT BROCKEL

#if CONF_CURRENT == BROCKEL

/*
 *      _____
 *   C /  E /
 *    /____/ F
 * B /  D /
 *  /____/G
 *    A   # P
 * 
 * {P,A,B,C,D,E,F,G}
 * 
 */
#define CONF_DISPLAY_CONVERT_TABLE {6,5,3,1,2,0,7,4}

/* PORTC only */
#define CONF_DISPLAY_ELEMENT_PINS {PC0, PC1, PC3, PC4}

/* PORTD only */
#define CONF_SHIFT_REG_PORT    (PORTD)
#define CONF_SHIFT_REG_SER_PIN (PD7)
#define CONF_SHIFT_REG_SCK_PIN (PD5)
#define CONF_SHIFT_REG_RCK_PIN (PD6)

#define CONF_SSEG_INVERT (TRUE)

#define CONF_GOAL_PULL_UPS_ENABLED (TRUE)

/* PORTD only */
#define CONF_BEEPER_PIN (PD4)
#define CONF_BEEPER_ON  (LOW)

/* PORTC only */
#define CONF_LED_PIN (PC5)

/* PIND only */
#define CONF_GOAL_PIN_0 (PD3)
#define CONF_GOAL_PIN_1 (PD2)

/* PINB only */
#define CONF_BUTTON_PIN_0 (PB3)
#define CONF_BUTTON_PIN_1 (PB4)
#define CONF_BUTTON_PIN_2 (PB0)
#define CONF_BUTTON_PIN_3 (PB2)
#define CONF_BUTTON_PIN_4 (PB1)

#elif CONF_CURRENT == BSCHWARTAU

/*
 *      _____
 *   C /  E /
 *    /____/ F
 * B /  D /
 *  /____/G
 *    A   # P
 * 
 * {P,A,B,C,D,E,F,G}
 * 
 */
#define CONF_DISPLAY_CONVERT_TABLE {6,3,7,4,2,1,0,5}

/* PORTC only */
#define CONF_DISPLAY_ELEMENT_PINS {PC3, PC4, PC5, PC6}

#define CONF_SHIFT_REG_PORT    (PORTC)
#define CONF_SHIFT_REG_SER_PIN (PC0)
#define CONF_SHIFT_REG_SCK_PIN (PC2)
#define CONF_SHIFT_REG_RCK_PIN (PC1)

#define CONF_SSEG_INVERT (TRUE)

#define CONF_GOAL_PULL_UPS_ENABLED (FALSE)

/* PORTD only */
#define CONF_BEEPER_PIN (PD4)
#define CONF_BEEPER_ON  (HIGH)

/* PORTC only */
#define CONF_LED_PIN (PC7)

/* PIND only */
#define CONF_GOAL_PIN_0 (PD2)
#define CONF_GOAL_PIN_1 (PD3)

/* PINB only */
#define CONF_BUTTON_PIN_0 (PB0)
#define CONF_BUTTON_PIN_1 (PB1)
#define CONF_BUTTON_PIN_2 (PB2)
#define CONF_BUTTON_PIN_3 (PB3)
#define CONF_BUTTON_PIN_4 (PB4)

#else

#error "current conf not supported"

#endif

/* checks */

#if CONF_BEEPER_ON != LOW && CONF_BEEPER_ON != HIGH
#error "CONF_BEEPER_ON must be LOW or HIGH"
#endif

#endif
