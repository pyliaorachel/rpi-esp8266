#ifndef UART_DEFS
#define UART_DEFS

void uart_init ( void );
int uart_getc ( void );
int uart_getc_timeout (char *b, unsigned timeout);
void uart_putc ( unsigned int c );

#endif
