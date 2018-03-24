#include "seven_seg.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <util/delay.h>

static uint8_t char_to_generic(char c);
static uint8_t convert(uint8_t v, seven_seg *sseg);

void seven_seg_init(seven_seg *sseg, uint8_t num_seg, mc_port port, shift_reg *seg_ano, mc_pin seg_cat[], uint8_t table[], boolean inverted)
{
    size_t n=0;

    assert(num_seg <= SEVEN_SEG_MAX_NUM_ELEM);

    /* set public */
    sseg->num_seg = num_seg;
    sseg->port = port;
    sseg->seg_ano = seg_ano;
    sseg->inverted = inverted;
    memset(sseg->seg_cat, 0, sizeof(sseg->seg_cat));
    memcpy(sseg->seg_cat, seg_cat, num_seg);
    memcpy(sseg->table, table, sizeof(sseg->table));

    /* set private */
    sseg->curr = 0;
    memset(sseg->val, 0, sizeof(sseg->val));

    for(;n<sseg->num_seg;++n)
    {
        *sseg->port &= ~(1<<sseg->seg_cat[n]);
    }
}

void seven_seg_set_chr(seven_seg *sseg, char val[])
{
    for(size_t n = 0; n < sseg->num_seg; ++n)
    {
        sseg->val[n] = convert(char_to_generic(val[n]), sseg);
    }
}

void seven_seg_set_raw(seven_seg *sseg, uint8_t val[])
{
    memcpy(sseg->val, val, sseg->num_seg);
}

void seven_seg_set_dot(seven_seg *sseg, uint8_t dots)
{
    int8_t n = sseg->num_seg - 1;
    const uint8_t dot = convert(0b10000000, sseg);

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
    *sseg->port &= ~(1<<sseg->seg_cat[sseg->curr]);

    /* next element */
    sseg->curr = (sseg->curr + 1) % sseg->num_seg;

    /* write configuration to shift reg */
    uint8_t val = sseg->val[sseg->curr];
    if(sseg->inverted)
        val = ~val;
    shift_reg_write(sseg->seg_ano,val);

    /* next element on */
    *sseg->port |= (1<<sseg->seg_cat[sseg->curr]);
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
        case 'A' : return 0b00111111;
        case 'b' :
        case 'B' : return 0b01111001;
        case 'c' :
        case 'C' : return 0b01101000;
        case 'd' :
        case 'D' : return 0b01101011;
        case 'e' :
        case 'E' : return 0b01111100;
        case 'f' :
        case 'F' : return 0b00111100;
        case 'g' :
        case 'G' : return 0b01110101;
        case 'h' : return 0b00111001;
        case 'H' : return 0b00111011;
        case 'i' : return 0b00000001;
        case 'I' : return 0b01100000;
        case 'j' : return 0b01000011;
        case 'J' : return 0b01100011;
        // k / K
        case 'l' :
        case 'L' : return 0b01110000;
        // m / M
        case 'n' :
        case 'N' : return 0b00101001;
        case 'o' :
        case 'O' : return 0b01101001;
        case 'p' :
        case 'P' : return 0b00111110;
        // q / Q
        case 'r' :
        case 'R' : return 0b00101000;
        // s / S
        case 't' :
        case 'T' : return 0b01111000;
        case 'u' : return 0b01100001;
        case 'U' : return 0b01110011;
        // v / V
        // w / W
        // x / X
        case 'y' :
        case 'Y' : return 0b00011011;
        // z / Z
        case '0' : return 0b01110111;
        case '1' : return 0b00000011;
        case '2' : return 0b01101110;
        case '3' : return 0b01001111;
        case '4' : return 0b00011011;
        case '5' : return 0b01011101;
        case '6' : return 0b01111101;
        case '7' : return 0b00000111;
        case '8' : return 0b01111111;
        case '9' : return 0b01011111;
        case '?' : return 0b00101110;
        // Not supported char
        default : return 0;
    }
}

static uint8_t convert(uint8_t v, seven_seg *sseg)
{
    uint8_t result = 0;
    uint8_t mask = 128;
    size_t n = 0;

    if(v)
    {
        for(;n < 8; ++n)
        {
            if(v & mask)
            {
                result |= (128 >> sseg->table[n]);
            }
            mask >>= 1;
        }
    }

    return result;
}
