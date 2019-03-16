#ifndef SW_UART_DEFS
#define SW_UART_DEFS

#define TX_SOFT 5
#define RX_SOFT 6
#define DELAY 104U // baudrate 9600

void sw_uart_init();

unsigned char sw_uart_getc();
unsigned char sw_uart_getc_robust(unsigned int robust_iter);
void sw_uart_putc(char c);
void sw_uart_putc_robust(unsigned char c, unsigned int robust_iter);

int sw_uart_readline(char *buf, unsigned sz);
void sw_uart_send_data(const char* data, unsigned nbytes);
void sw_uart_writeline(const char* data);

#endif