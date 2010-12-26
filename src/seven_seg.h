#ifndef SEVEN_SEG_H
#define SEVEN_SEG_H

#include <stdint.h>

#include "shift_reg.h"

typedef volatile uint8_t* seven_seg_port;
typedef volatile uint8_t seven_seg_pin;

#define SEVEN_SEG_MAX_NUM_SEG (4)

struct seven_seg_t
{
    uint8_t num_seg;
    seven_seg_port port;
    
    shift_reg *seg_ano;
    seven_seg_pin seg_cat[SEVEN_SEG_MAX_NUM_SEG];
    
    //private
    uint8_t curr;
    uint8_t val[SEVEN_SEG_MAX_NUM_SEG];
    uint8_t pause;
};

typedef struct seven_seg_t seven_seg;

void seven_seg_init(seven_seg *sseg);
void seven_seg_set_val(seven_seg *sseg, uint8_t val);
void seven_seg_loop(seven_seg *sseg);

#endif
