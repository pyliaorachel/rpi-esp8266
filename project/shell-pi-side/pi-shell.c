#include "rpi.h"
#include "pi-shell.h"

#define TX_SOFT 5
#define RX_SOFT 6
#define HIGH 1
#define LOW 0
static const char pi_done[] = "PI REBOOT!!!";

// read characters until we hit a newline.
static int readline(char *buf, int sz) {
	for(int i = 0; i < sz; i++) {
		if((buf[i] = uart_getc()) == '\n') {
			buf[i] = 0;
			return i;
		}
	}
	// just return partial read?
	panic("size too small\n");
}

void notmain() { 
	uart_init();
	int n;
	char buf[1024];

	while((n = readline(buf, sizeof buf))) {
		if (strncmp(buf, "echo ", 4) == 0) {
			printk("%s\n", buf);
        } else if (strncmp(buf, "reboot", 6) == 0) {
            printk("%s\n", pi_done);
	        delay_ms(100);  
            clean_reboot();
        } else if (strncmp(buf, "run", 3) == 0) {
            unsigned addr = load_code();
            BRANCHTO(addr);
		} else if (strncmp(buf, "esp", 3) == 0) {
            printk("esp\n");
        }
	}
	clean_reboot();
}
