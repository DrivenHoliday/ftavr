#ifndef SEVEN_SEG_H
#define SEVEN_SEG_H

#include <stdint.h>

#include "shift_reg.h"
#include "types.h"

#define SEVEN_SEG_MAX_NUM_SEG (4)

struct seven_seg_t
{
    uint8_t num_seg;
    mc_port port;
    
    shift_reg *seg_ano;
    mc_pin seg_cat[SEVEN_SEG_MAX_NUM_SEG];
    
    uint8_t table[8];
    boolean inverted;
    
    //private
    uint8_t curr;
    uint8_t val[SEVEN_SEG_MAX_NUM_SEG];
};

typedef struct seven_seg_t seven_seg;

void seven_seg_init(seven_seg *sseg, uint8_t num_seg, mc_port port, shift_reg *seg_ano, mc_pin seg_cat[], uint8_t table[], boolean inverted);
void seven_seg_set_chr(seven_seg *sseg, char val[]);
void seven_seg_set_val(seven_seg *sseg, uint8_t val[]);
void seven_seg_loop(seven_seg *sseg);

uint8_t char_to_generic(char c);
uint8_t convert(uint8_t v, uint8_t table[]);

#endif
