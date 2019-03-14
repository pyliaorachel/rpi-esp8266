void sw_uart_init();
unsigned char sw_uart_getc();
void sw_uart_putc(char c);
int sw_uart_readline(char *buf, unsigned sz);
void sw_uart_send_data(const char* data, unsigned nbytes);
void sw_uart_writeline(const char* data);