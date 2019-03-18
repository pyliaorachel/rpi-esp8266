#include "rpi.h"
#include "pi-shell.h"

static const char pi_done[] = "PI REBOOT!!!";

// read characters until we hit a newline.
static int readline(char *buf, int sz) {
	for(int i = 0; i < sz; i++) {
		if((buf[i] = sw_uart_getc()) == '\n') {
			buf[i] = 0;
			return i;
		}
	}
	panic("size too small\n");
}

static int esp_readline(char *buf, int sz) {
	for(int i = 0; i < sz; i++) {
		if((buf[i] = uart_getc()) == '\n') {
			buf[i] = 0;
			return i;
		}
	}
	panic("size too small\n");
}

// read characters until we hit a \r\n. Include \r\n.
static int read_CRLF_line(char *buf, int sz) {
	for(int i = 0; i < sz-1; i++) { // leave one byte for \0
		if ((buf[i] = sw_uart_getc()) == '\n') {
			if (buf[i-1] == '\r') {
			  buf[i+1] = 0;
				return i+1;
			}
		}
	}
	panic("size too small\n");
}

// read characters until we hit a \r\n. Include \r\n.
static int esp_read_CRLF_line(char *buf, int sz) {
	for(int i = 0; i < sz-1; i++) { // leave one byte for \0
		if ((buf[i] = uart_getc()) == '\n') {
			if (buf[i-1] == '\r') {
			  buf[i+1] = 0;
			  return i+1;
			}
		}
	}
	panic("size too small\n");
}

void notmain() { 
	uart_init();
	sw_uart_init();
   
	int n;
	char buf[1024];
	char c;
	
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
			while (c = uart_getc()); // ESP should reset here
			printk("Welcome!\n");
			
			while (n = read_CRLF_line(buf, sizeof buf)) {
			  printk("buffer: %s\n", buf);
			  for (size_t i = 0; i < n; i++) {
			    uart_putc(buf[i]);
			  }
				printk("written\n");
				if (n = esp_read_CRLF_line(buf, sizeof buf)) {
				  printk("received <%s>, <%d> bytes\n", buf, n);
				}
				printk("END\n");
			}
			
		}
	}
	clean_reboot();
}
