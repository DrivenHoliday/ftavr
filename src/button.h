#ifndef BUTTON_H
#define BUTTON_H

#include <stddef.h>

#include "types.h"

#ifndef BUTTON_MAX_NUM_BUTTON
#define BUTTON_MAX_NUM_BUTTON (10)
#endif

typedef void(*button_func)(void*);

struct button_s
{
    mc_port port;
    mc_pin pin;
    button_func func;
    void *payload;
    boolean status;
};

typedef struct button_list_s
{
    struct button_s buttons[BUTTON_MAX_NUM_BUTTON];
    size_t num;
} button_list;


void button_init(button_list *butt);
void button_add(button_list *butt, mc_port port, mc_pin pin, button_func func, void *payload);
void button_poll(button_list *butt);
void button_poll_action(button_list *butt, boolean action);

#endif
