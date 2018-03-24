#include "seven_seg.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <util/delay.h>

#include "config.h"

static uint8_t char_to_generic(char c);

void seven_seg_init(seven_seg *sseg, uint8_t num_seg, mc_port port, shift_reg *seg_ano, mc_pin seg_cat[], boolean inverted)
{
    size_t n=0;

    assert(num_seg <= SEVEN_SEG_MAX_NUM_ELEM);

    /* set public */
    sseg->num_seg = num_seg;
    sseg->port = port;
    sseg->seg_ano = seg_ano;
    sseg->inverted = inverted;

    /* set private */
    sseg->curr = 0;
    memset(sseg->val, 0, sizeof(sseg->val));

    for(;n<sseg->num_seg;++n)
    {
        sseg->seg_cat[n] = 1 << seg_cat[n];
        *sseg->port &= ~sseg->seg_cat[n];
    }
}

void seven_seg_set_chr(seven_seg *sseg, char val[])
{
    for(size_t n = 0; n < sseg->num_seg; ++n)
    {
        sseg->val[n] = char_to_generic(val[n]);
    }
}

void seven_seg_set_raw(seven_seg *sseg, uint8_t val[])
{
    memcpy(sseg->val, val, sseg->num_seg);
}

void seven_seg_set_dot(seven_seg *sseg, uint8_t dots)
{
    int8_t n = sseg->num_seg - 1;
    const uint8_t dot = CONVERT_SEGMENT(0b10000000);

    for(;n >= 0; --n)
    {
        if(dots & 1)
        {
            sseg->val[n] |= dot;
        }
        else
        {
            sseg->val[n] &= ~dot;
        }

        dots >>= 1;
    }
}

void seven_seg_loop(seven_seg *sseg)
{
    /* current element off */
    *sseg->port &= ~sseg->seg_cat[sseg->curr];

    /* next element */
    sseg->curr = (sseg->curr + 1) % sseg->num_seg;

    /* write configuration to shift reg */
    uint8_t val = sseg->val[sseg->curr];
    if(sseg->inverted)
        val = ~val;
    shift_reg_write(sseg->seg_ano,val);

    /* next element on */
    *sseg->port |= sseg->seg_cat[sseg->curr];
}

/**
 * Converts a char into a generic seven segment value.
 *
 * Returns a unsigned 8 bit wide integer with following layout:
 * 0bPABCDEFG
 *      _____
 *   C /  E /
 *    /____/ F
 * B /  D /
 *  /____/G
 *    A   # P
 *
 * It already applies the conversion to the specific configured mapping.
 */
/*
 * Not supported chars:
 * k, m, s, q, v, w, x, z
 */
static uint8_t char_to_generic(char c)
{
    switch (c)
    {
        case 'a' :
        case 'A' : return CONVERT_SEGMENT(0b00111111);
        case 'b' :
        case 'B' : return CONVERT_SEGMENT(0b01111001);
        case 'c' :
        case 'C' : return CONVERT_SEGMENT(0b01101000);
        case 'd' :
        case 'D' : return CONVERT_SEGMENT(0b01101011);
        case 'e' :
        case 'E' : return CONVERT_SEGMENT(0b01111100);
        case 'f' :
        case 'F' : return CONVERT_SEGMENT(0b00111100);
        case 'g' :
        case 'G' : return CONVERT_SEGMENT(0b01110101);
        case 'h' : return CONVERT_SEGMENT(0b00111001);
        case 'H' : return CONVERT_SEGMENT(0b00111011);
        case 'i' : return CONVERT_SEGMENT(0b00000001);
        case 'I' : return CONVERT_SEGMENT(0b01100000);
        case 'j' : return CONVERT_SEGMENT(0b01000011);
        case 'J' : return CONVERT_SEGMENT(0b01100011);
        // k / K
        case 'l' :
        case 'L' : return CONVERT_SEGMENT(0b01110000);
        // m / M
        case 'n' :
        case 'N' : return CONVERT_SEGMENT(0b00101001);
        case 'o' :
        case 'O' : return CONVERT_SEGMENT(0b01101001);
        case 'p' :
        case 'P' : return CONVERT_SEGMENT(0b00111110);
        // q / Q
        case 'r' :
        case 'R' : return CONVERT_SEGMENT(0b00101000);
        // s / S
        case 't' :
        case 'T' : return CONVERT_SEGMENT(0b01111000);
        case 'u' : return CONVERT_SEGMENT(0b01100001);
        case 'U' : return CONVERT_SEGMENT(0b01110011);
        // v / V
        // w / W
        // x / X
        case 'y' :
        case 'Y' : return CONVERT_SEGMENT(0b00011011);
        // z / Z
        case '0' : return CONVERT_SEGMENT(0b01110111);
        case '1' : return CONVERT_SEGMENT(0b00000011);
        case '2' : return CONVERT_SEGMENT(0b01101110);
        case '3' : return CONVERT_SEGMENT(0b01001111);
        case '4' : return CONVERT_SEGMENT(0b00011011);
        case '5' : return CONVERT_SEGMENT(0b01011101);
        case '6' : return CONVERT_SEGMENT(0b01111101);
        case '7' : return CONVERT_SEGMENT(0b00000111);
        case '8' : return CONVERT_SEGMENT(0b01111111);
        case '9' : return CONVERT_SEGMENT(0b01011111);
        case '?' : return CONVERT_SEGMENT(0b00101110);
        case '-' : return CONVERT_SEGMENT(0b00001000);
        case '_' : return CONVERT_SEGMENT(0b01000000);
        // Not supported char
        default : return 0;
    }
}
