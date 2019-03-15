#include "rpi.h"

// as you can see, b/c we have no OS protection, we have to change code
// to prevent it from killing the machine.  we will add more and more
// tricks to prevent this vulnerability.
void notmain(void) {
	// NB: we can't do this b/c the shell already initialized and resetting
	// uart may reset connection to Unix.
  //sw_uart_init();

	// if not working, try just printing characters.
  #if 0
	sw_uart_putc('h');
	sw_uart_putc('e');
	sw_uart_putc('l');
	sw_uart_putc('l');
	sw_uart_putc('o');

	sw_uart_putc(' ');

	sw_uart_putc('w');
	sw_uart_putc('o');
	sw_uart_putc('r');
	sw_uart_putc('l');
	sw_uart_putc('d');

	sw_uart_putc('\n');
#endif

	printk("hello world\n");

	return;

	// printk("hello world from address %p\n", (void*)notmain);


	// NB: this is supposed to be a thread_exit().  calling reboot will
	// kill the pi.
	// clean_reboot();
}
