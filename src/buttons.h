#ifndef BUTTONS_H
#define BUTTONS_H

#include "types.h"

struct buttons_t {

    mc_pin down_1;
    mc_pin up_1;
    mc_pin down_2;
    mc_pin up_2;
    mc_pin foobar;
};

typedef struct buttons_t buttons;

#endif BUTTONS_H
