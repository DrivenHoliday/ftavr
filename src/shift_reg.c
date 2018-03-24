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

static void set(shift_reg *reg, uint8_t b)
{
    if(!b)
    {
        *reg->port |= reg->ser;
    }
    else
    {
        *reg->port &= ~reg->ser;
    }

    pulse(reg->port, reg->sck);
}

void shift_reg_write(shift_reg *reg, uint8_t value)
{
    *reg->port &= ~( reg->ser | reg->sck | reg->rck );

    set(reg,value&128);
    set(reg,value&64);
    set(reg,value&32);
    set(reg,value&16);
    set(reg,value&8);
    set(reg,value&4);
    set(reg,value&2);
    set(reg,value&1);

    pulse(reg->port, reg->rck);
}
