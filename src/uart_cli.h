#ifndef UART_CLI_H
#define UART_CLI_H

typedef void(*uart_cli_func)(void*,void*);

enum uart_cli_param_type_e
{
    UART_CLI_VOID = 0,
    UART_CLI_INT,
};

typedef enum uart_cli_param_type_e uart_cli_param_type;

void uart_cli_init(void);
void uart_cli_add_cmd(char c, const char *desc, uart_cli_param_type param, uart_cli_func func, void *payload);
void uart_cli_print_help(void);
void uart_cli_proc(void);

#endif
