#include "rpi.h"
#include "pi-shell.h"

#define TIMEOUT 10000000
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

// read characters from ESP until we hit a newline.
static int esp_readline(char *buf, int sz) {
	for(int i = 0; i < sz; i++) {
		if((buf[i] = uart_getc()) == '\n') {
			buf[i] = 0;
			return i;
		}
	}
	panic("size too small\n");
}

// read characters until we hit a \r\n.
static int read_CRLF_line(char *buf, int sz) {
	for(int i = 0; i < sz; i++) {
		if ((buf[i] = sw_uart_getc()) == '\n') {
			if (buf[i-1] == '\r') {
                buf[i-1] = 0;
                return i-1;
			}
		}
	}
	panic("size too small\n");
}

// read characters from ESP until we hit a \r\n.
static int esp_read_CRLF_line(char *buf, int sz) {
	for(int i = 0; i < sz; i++) {
        unsigned ret = uart_getc_timeout(&buf[i], TIMEOUT);
        if (ret == 1) {
            if (buf[i] == '\n') {
                if (buf[i-1] == '\r') {
                    buf[i-1] = 0;
                    return i-1;
                }
            }
        } else {
            printk("read CRLF timeout after %d\n", ret);
            buf[i] = 0;
            return i;
        }
	}
	panic("size too small\n");
}

// read characters to ESP, include \r\n.
void esp_write_CRLF_line(char *buf, int nbytes) {
    for (int i = 0; i < nbytes; i++) {
        uart_putc(buf[i]);
        delay_ms(1);
    }
    uart_putc('\r'); uart_putc('\n');
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
                printk("%s\n", buf);

                // Forward data to ESP
                esp_write_CRLF_line(buf, n);

                // Read response from ESP
				if (n = esp_read_CRLF_line(buf, sizeof buf)) {
                    printk("%s\n", buf);
                }

                // Let the other end know we're done
				printk("END\n");
			}
			
		}
	}
	clean_reboot();
}
