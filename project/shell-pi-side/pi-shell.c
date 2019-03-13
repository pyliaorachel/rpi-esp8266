#include "rpi.h"
#include "spi.h"
#include "pi-shell.h"

#define TX_SOFT 5
#define RX_SOFT 6
#define HIGH 1
#define LOW 0
static const char pi_done[] = "PI REBOOT!!!";
void soft_uart_init(unsigned char txpin){
    gpio_set_output(txpin);
    gpio_write(txpin, HIGH);
    gpio_set_input(rxpin);
    gpio_write(rxpin, HIGH);
}
void soft_uart_tx(unsigned char txpin, unsigned us_per_bit, const char* data, unsigned nbytes) {
    for (unsigned i = 0; i < nbytes; i++) {
        char c = data[i];

        gpio_write(txpin, LOW);
        delay_us(us_per_bit);

        for (unsigned bit = 0; bit < (sizeof(char) * 8); bit++) {
            gpio_write(txpin, (unsigned char) ((c >> bit) & 1));
            delay_us(us_per_bit);
        }

        gpio_write(txpin, HIGH);
        delay_us(us_per_bit);
    }
}

void soft_uart_rx(unsigned char rxpin, unsigned us_per_bit, const char *buffer) {
    for (unsigned i=0; i < )

}

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
            char data[] = "gpio.mode(3, gpio.OUTPUT)\r\ngpio.write(3, gpio.LOW)\r\n";
            unsigned nbytes = sizeof(data);
            soft_uart_init();
            soft_uart_tx(TX_SOFT, 104U, data, nbytes);
        }
	}
	clean_reboot();
}
