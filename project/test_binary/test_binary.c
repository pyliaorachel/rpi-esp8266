#include "rpi.h"


void notmain() {
  sw_uart_init();

  sw_uart_putc('h');
  sw_uart_putc('e');
  sw_uart_putc('l');
  sw_uart_putc('l');
  sw_uart_putc('o');
  sw_uart_putc('\n');

  clean_reboot();
}

