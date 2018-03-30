#include "button.h"

#include <assert.h>
#include <string.h>

void button_init(button_list *butt)
{
    memset(butt, 0, sizeof(button_list));
}

void button_add(button_list *butt, mc_port port, mc_pin pin, button_func func, void *payload)
{
    butt->buttons[butt->num].port = port;
    butt->buttons[butt->num].pin = pin;
    butt->buttons[butt->num].func = func;
    butt->buttons[butt->num].payload = payload;
    butt->buttons[butt->num].status = (*butt->buttons[butt->num].port & (1<<butt->buttons[butt->num].pin));

    ++butt->num;
    assert(butt->num <= BUTTON_MAX_NUM_BUTTON);
}

void button_poll_action(button_list *butt, boolean action)
{
    size_t n = 0;
    boolean curr;

    for(;n < butt->num; ++n)
    {
        curr = *butt->buttons[n].port & (1<<butt->buttons[n].pin);
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
