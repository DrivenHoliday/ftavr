#ifndef SEVEN_SEG_H
#define SEVEN_SEG_H

#include <stdint.h>

#include "shift_reg.h"
#include "types.h"

#ifndef SEVEN_SEG_MAX_NUM_ELEM
#define SEVEN_SEG_MAX_NUM_ELEM (4)
#endif

#if SEVEN_SEG_MAX_NUM_ELEM > 8
#error "More then 8 elements arn't supported"
#endif

struct seven_seg_s
{
    uint8_t num_seg;
    mc_port port;

    shift_reg *seg_ano;
    mc_pin seg_cat[SEVEN_SEG_MAX_NUM_ELEM];

    boolean inverted;

    //private
    uint8_t curr;
    uint8_t val[SEVEN_SEG_MAX_NUM_ELEM];
};

typedef struct seven_seg_s seven_seg;

void seven_seg_init(seven_seg *sseg, uint8_t num_seg, mc_port port, shift_reg *seg_ano, mc_pin seg_cat[], boolean inverted);

void seven_seg_set_chr(seven_seg *sseg, char val[]);
void seven_seg_set_raw(seven_seg *sseg, uint8_t val[]);
void seven_seg_set_dot(seven_seg *sseg, uint8_t dots);


void seven_seg_loop(seven_seg *sseg);

#endif
