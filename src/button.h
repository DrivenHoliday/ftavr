#ifndef BUTTON_H
#define BUTTON_H

#include <stddef.h>

#include "types.h"

#ifndef BUTTON_MAX_NUM_BUTTON
#define BUTTON_MAX_NUM_BUTTON (10)
#endif

typedef void(*button_func)(void*);

struct button_single_button_s
{
    mc_port port;
    mc_pin pin;
    button_func func;
    void *payload;
    boolean status;
};

struct button_s
{
    struct button_single_button_s buttons[BUTTON_MAX_NUM_BUTTON];
    size_t num;
};

typedef struct button_s button;

void button_init(button *butt);
void button_add(button *butt, mc_port port, mc_pin pin, button_func func, void *payload);
void button_poll(button *butt);

#endif
