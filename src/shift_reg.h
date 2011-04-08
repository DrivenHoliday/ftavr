#ifndef SHIFT_REG_H
#define SHIFT_REG_H

#include <stdint.h>

#include "types.h"

struct shift_reg_s
{
    mc_port port;
    mc_pin ser;
    mc_pin sck;
    mc_pin rck;
};

typedef struct shift_reg_s shift_reg;

void shift_reg_init(shift_reg *reg, mc_port port, mc_pin ser, mc_pin sck, mc_pin rck);
void shift_reg_write(shift_reg *reg, uint8_t value);

#endif
