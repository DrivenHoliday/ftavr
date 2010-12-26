#include "seven_seg.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

void seven_seg_init(seven_seg *sseg)
{
    uint8_t n=0;

    assert(sseg->num_seg <= SEVEN_SEG_MAX_NUM_SEG);

    sseg->curr = 0;

    for(;n<sseg->num_seg;++n)
    {
        sseg->val[n] = 0;
        *sseg->port &= ~(1<<sseg->seg_cat[n]);
    }
}

/*
  0b0defgabc
 */
static const uint8_t nto7seg[] = {
                                 0b01110111,
                                 0b00000011,
                                 0b01101110,
                                 0b01001111,
                                 0b00011011,
                                 0b01011101,
                                 0b01111101,
                                 0b00000111,
                                 0b01111111,
                                 0b00011111};

void seven_seg_set_val(seven_seg *sseg, uint8_t val)
{
    uint8_t vn=0,sn;

    static char str[SEVEN_SEG_MAX_NUM_SEG];
    itoa(val, str, 10);
    sn = strlen(str);

    for(;vn<sseg->num_seg;++vn)
    {
        if(sn)
            sseg->val[vn] = nto7seg[str[--sn]-48];
        else
            sseg->val[vn] = 0;
    }
}

void seven_seg_loop(seven_seg *sseg)
{
    *sseg->port &= ~(1<<sseg->seg_cat[sseg->curr]);

    sseg->curr += 1;
    if(!(sseg->curr%sseg->num_seg)) sseg->curr = 0;

    if(sseg->val[sseg->curr])
    {
        shift_reg_write(sseg->seg_ano,sseg->val[sseg->curr]);

        *sseg->port |= (1<<sseg->seg_cat[sseg->curr]);

    }
}
