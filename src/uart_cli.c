#include "uart_cli.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "uart.h"

#ifndef UART_CLI_MAX_NUM_CMD
#define UART_CLI_MAX_NUM_CMD (10)
#endif

struct uart_cli_cmd_s
{
    char c;
    const char *desc;
    uart_cli_param_type param;
    uart_cli_func func;
    void *payload;
};

typedef struct uart_cli_cmd_s uart_cli_cmd;

static uart_cli_cmd cmd[UART_CLI_MAX_NUM_CMD];
static size_t num_cmd = 0;
static char cmd_buf[32];

void uart_cli_init(void)
{
    memset(cmd, 0, sizeof(cmd));
    num_cmd = 0;
    memset(cmd_buf, 0, sizeof(cmd_buf));
}

void uart_cli_add_cmd(char c, const char *desc, uart_cli_param_type param, uart_cli_func func, void *payload)
{
    assert(num_cmd < UART_CLI_MAX_NUM_CMD);
    
    cmd[num_cmd].c = c;
    cmd[num_cmd].desc = desc;
    cmd[num_cmd].param = param;
    cmd[num_cmd].func = func;
    cmd[num_cmd].payload = payload;
    
    num_cmd += 1;
}

static void uart_cli_parse_cmd(void)
{
    uart_buf_puts("uart_cli_parse_cmd\n");
}

void uart_cli_proc(void)
{
    char c = 0;
    
    uart_buf_puts("uart_cli_proc\n");
    
    if(uart_getc(c))
    {
        uart_buf_putc(c);
        if(c == '\n')
        {
            uart_cli_parse_cmd();
            memset(cmd_buf, 0, sizeof(cmd_buf));
        }
        else
        {
            strncat(cmd_buf, &c, 1);
        }
    }
    else
        uart_buf_putc('g');
}

void uart_cli_print_help(void)
{
}
