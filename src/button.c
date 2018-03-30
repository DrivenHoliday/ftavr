#include "button.h"

#include <assert.h>
#include <string.h>

#define BUTTON_STATUS(button) (*(button).port & (button).pin)

void button_init(button_list *butt)
{
    memset(butt, 0, sizeof(button_list));
}

void button_add(button_list *butt, mc_port port, mc_pin pin, button_func func, void *payload)
{
    struct button_s *new_button = &butt->buttons[butt->num];
    new_button->port = port;
    new_button->pin = 1 << pin;
    new_button->func = func;
    new_button->payload = payload;
    new_button->status = BUTTON_STATUS(*new_button);

    ++butt->num;
    assert(butt->num <= BUTTON_MAX_NUM_BUTTON);
}

void button_poll_action(button_list *butt, boolean action)
{
    size_t n = 0;
    boolean curr;

    for(;n < butt->num; ++n)
    {
        curr = BUTTON_STATUS(butt->buttons[n]);
        if(curr && !butt->buttons[n].status && action)
        {
            (*butt->buttons[n].func)(butt->buttons[n].payload);
        }
        butt->buttons[n].status = curr;
    }
}

void button_poll(button_list *butt)
{
    button_poll_action(butt, TRUE);
}
