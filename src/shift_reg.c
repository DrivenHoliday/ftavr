#include "shift_reg.h"

void shift_reg_init(shift_reg *reg, mc_port port, mc_pin ser, mc_pin sck, mc_pin rck)
{
    reg->port = port;
    reg->ser = 1 << ser;
    reg->sck = 1 << sck;
    reg->rck = 1 << rck;

    shift_reg_write(reg, 0);
}

static void pulse(mc_port port, mc_pin pin)
{
    *port |= pin;
    *port &= ~pin;
}

void shift_reg_write(shift_reg *reg, uint8_t value)
{
    *reg->port &= ~( reg->ser | reg->sck | reg->rck );

    for (uint8_t mask = 0x80; mask > 0; mask >>= 1) {
        if (value & mask)
        {
            *reg->port &= ~reg->ser;
        }
        else
        {
            *reg->port |= reg->ser;
        }
        pulse(reg->port, reg->sck);
    }

    pulse(reg->port, reg->rck);
}
