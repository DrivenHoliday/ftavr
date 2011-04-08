#include <math.h>

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>

#define BAUD 19200
#include <util/setbaud.h>

#include "shift_reg.h"
#include "seven_seg.h"
#include "uart.h"
#include "types.h"

static seven_seg sseg;

int main(void)
{
    shift_reg reg;

//TODO: Use correct C syntax
    char play[] = "Play";
    
    DDRC  = 0xff;
    PORTC = 0x0;

    shift_reg_init(&reg, &PORTC, PC2, PC1, PC0);
    
    mc_pin seg_cat[] = { PC3, PC4, PC5, PC6 };
    uint8_t table[] = {0, 1, 2, 3, 4, 5, 6, 7};
    
    seven_seg_init(&sseg, 4, &PORTC, &reg, seg_cat, table);

    
    seven_seg_set_chr(&sseg, play);
    
    while(1)
    {
        seven_seg_loop(&sseg);
    }

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
