#ifndef SHIFT_REG_H
#define SHIFT_REG_H

#include <stdint.h>

typedef volatile uint8_t* shift_reg_port;
typedef volatile uint8_t shift_reg_pin;

struct shift_reg_t
{
    shift_reg_port port;
    shift_reg_pin ser;
    shift_reg_pin sck;
    shift_reg_pin rck;
};

typedef struct shift_reg_t shift_reg;

void shift_reg_init(shift_reg *reg, shift_reg_port port, shift_reg_pin ser, shift_reg_pin sck, shift_reg_pin rck);
void shift_reg_write(shift_reg *reg, uint8_t value);

#endif
