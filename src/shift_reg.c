#include "shift_reg.h"

void shift_reg_init(shift_reg *reg, shift_reg_port port, shift_reg_pin ser, shift_reg_pin sck, shift_reg_pin rck)
{
    reg->port = port;
    reg->ser = ser;
    reg->sck = sck;
    reg->rck = rck;
    
    shift_reg_write(reg,0); 
}

static void pulse(shift_reg_port port, shift_reg_pin pin)
{
    *port |= (1<<pin);
    *port &= ~(1<<pin);
}

static void set(shift_reg *reg, uint8_t b)
{
    if(!b)
    {
        *reg->port |= (1<<reg->ser);
    }
    else
    {
        *reg->port &= ~(1<<reg->ser);
    }
    
    pulse(reg->port, reg->sck);        
}

void shift_reg_write(shift_reg *reg, uint8_t value)
{
    *reg->port &= ~( (1<<reg->ser) | (1<<reg->sck) | (1<<reg->rck) );
    
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
