#include "button.h"

#include <assert.h>
#include <string.h>

void button_init(button *butt)
{
    memset(butt, 0, sizeof(button));
}

void button_add(button *butt, mc_port port, mc_pin pin, button_func func, void *payload)
{
    butt->buttons[butt->num].port = port;
    butt->buttons[butt->num].pin = pin;
    butt->buttons[butt->num].func = func;
    butt->buttons[butt->num].payload = payload;
    
    ++butt->num;
    assert(butt->num <= BUTTON_MAX_NUM_BUTTON);
}

void button_poll(button *butt)
{
    size_t n = 0;
    boolean curr;
    
    for(;n < butt->num; ++n)
    {
        curr = *butt->buttons[n].port & (1<<butt->buttons[n].pin);
        if(curr && !butt->buttons[n].status)
        {
            (*butt->buttons[n].func)(butt->buttons[n].payload);
        }
        butt->buttons[n].status = curr;
    }
}
