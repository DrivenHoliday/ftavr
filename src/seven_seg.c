#include "seven_seg.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

void seven_seg_init(seven_seg *sseg, uint8_t num_seg, mc_port port, shift_reg *seg_ano, mc_pin seg_cat[], uint8_t table[])
{
    sseg->num_seg = num_seg;
    sseg->port = port;
    sseg->seg_ano = seg_ano;
    //TODO: cat & table?

    uint8_t n=0;

    assert(sseg->num_seg <= SEVEN_SEG_MAX_NUM_SEG);

    sseg->curr = 0;

    for(;n<sseg->num_seg;++n)
    {
        sseg->val[n] = 0;
        *sseg->port &= ~(1<<sseg->seg_cat[n]);
    }
}

void seven_seg_set_chr(seven_seg *sseg, char val[])
{    
    for (int i = 0; i < sseg->num_seg; i++)
    {
        sseg->val[i] = convert(char_to_generic(val[i]), sseg->table);
    }
}

void seven_seg_set_val(seven_seg *sseg, uint8_t val[])
{
    for (int i = 0; i < sseg->num_seg; i++)
    {
        sseg->val[i] = val[i];
    }
}

void seven_seg_loop(seven_seg *sseg)
{
    uint8_t cnt = sseg->num_seg;
    do
    {
        if (seven_seg_loop_int(sseg)) {
            return;
        }
    } while (cnt);
}

int seven_seg_loop_int(seven_seg *sseg)
{
    *sseg->port &= ~(1<<sseg->seg_cat[sseg->curr]);

    sseg->curr = (sseg->curr + 1) % sseg->num_seg;

    if(sseg->val[sseg->curr])
    {
        shift_reg_write(sseg->seg_ano,sseg->val[sseg->curr]);

        *sseg->port |= (1<<sseg->seg_cat[sseg->curr]);
        return 1;
    }
    return 0;
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
 * k, m, s, q, w, z
 */
uint8_t char_to_generic(char c)
{
    switch (c)
    {
    case 'a' :
    case 'A' : return 0b00110111;
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
    case 'H' : return 0b00111011; // Conflict x/X
    case 'i' : return 0b00000001;
    case 'I' : return 0b01100000; // Conflict l
    case 'j' : return 0b01000011;
    case 'J' : return 0b01100011;
    // k / K
    case 'l' : return 0b00110000; // Conflict I
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
    case 'R' : return 0b01010000;
    // s / S
    case 't' :
    case 'T' : return 0b01111000;
    case 'u' : return 0b01100001; // Conflict v
    case 'U' : return 0b01110011; // Conflict V
    case 'v' : return 0b01100001; // Conflict u
    case 'V' : return 0b01110011; // Conflict U
    // w / W
    case 'x' :
    case 'X' : return 0b00111011; // Conflict H
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
    // Not supported char
    default : return 0;
    }
}

uint8_t convert(uint8_t v, uint8_t table[])
{
    uint8_t result = 0;
    if (v)
    {
        uint8_t mask = 1;
        for (int i = 0; i < 8; i++)
        {
            if (v & mask)
            {
                result |= (1 << table[i]);
            }
            mask <<= 1;
        }
    }
    return result;
}
