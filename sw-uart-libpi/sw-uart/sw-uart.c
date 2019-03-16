#include "gpio-int/gpio-int.h"

#define TX_SOFT 5
#define RX_SOFT 6
#define DELAY 104U // baudrate 9600

void int_handler(unsigned pc) {
  if (gpio_event_detected(RX_SOFT)) {
    //unsigned char data = sw_uart_read_byte();
    gpio_event_clear(RX_SOFT);
  }
}

unsigned char sw_uart_read_byte() {
  unsigned char c = 0;
  delay_us(DELAY);

  // Latch 8 bits of data
  for (unsigned bit = 0; bit < 8; bit++) {
    unsigned data = gpio_read(RX_SOFT);
    c |= data << bit;
    delay_us(DELAY);
  }
  // End bit
  gpio_read(RX_SOFT);
  return c;
}

unsigned char sw_uart_getc() {
  // TODO: timeout
  while (gpio_read(RX_SOFT) != LOW)
    delay_us(DELAY >> 1); // critical! don't wait for too long
  return sw_uart_read_byte(); 
}

void sw_uart_putc(char c) {
  gpio_write(TX_SOFT, LOW);
  delay_us(DELAY);
  for (unsigned bit = 0; bit < 8; bit++) {
    gpio_write(TX_SOFT, (c >> bit) & 1);
    delay_us(DELAY);
  }
  gpio_write(TX_SOFT, HIGH);
  delay_us(DELAY);
}

int sw_uart_readline(char *buf, unsigned sz) {
  for (int i = 0; i < sz; i++) {
		buf[i] = sw_uart_getc();
		if (buf[i] == '\n') {
			buf[i] = 0;
			return i;
		}
  }
  return -1;
}

void sw_uart_send_data(const char* data, unsigned nbytes) {
  for (unsigned i = 0; i < nbytes; i++) {
    char c = data[i];
    sw_uart_putc(c);
  }
}

void sw_uart_writeline(const char* data) {
  int i = 0;
  while (data[i] != '\n') {
    char c = data[i];
    sw_uart_putc(c);
  }
  sw_uart_putc('\n');
}

void sw_uart_init_rx() {
  gpio_set_input(RX_SOFT);
  gpio_set_pullup(RX_SOFT);
  //gpio_int_init(RX_SOFT, FALLING_EDGE);
}

void sw_uart_init_tx() {
  gpio_set_output(TX_SOFT);
  gpio_set_pullup(TX_SOFT);
  gpio_write(TX_SOFT, HIGH);
}

void sw_uart_init() {
  sw_uart_init_rx();
  sw_uart_init_tx();
}
