#include "gpio-int/gpio-int.h"
#include "sw-uart.h"

/* MISC */

// Wait for GPIO falling edge interrupt to trigger read
// not used now, busy-waiting implemented instead
void int_handler(unsigned pc) {
  if (gpio_event_detected(RX_SOFT)) {
    //unsigned char data = sw_uart_getc_ll();
    gpio_event_clear(RX_SOFT);
  }
}

/* I/O */

// Low-level get char
unsigned char sw_uart_getc_ll() {
  unsigned char c = 0;
  // Start bit detected, wait for the bit to pass
  delay_us(DELAY);

  // Latch 8 bits of data
  for (unsigned bit = 0; bit < 8; bit++) {
    unsigned data = gpio_read(RX_SOFT);
    c |= data << bit;
    delay_us(DELAY);
  }
  // End bit
  gpio_read(RX_SOFT);
  // not sure why we don't need delay here.
  return c;
}

// High-level get char
unsigned char sw_uart_getc() {
  // TODO: timeout
  while (gpio_read(RX_SOFT) != LOW)
    delay_us(DELAY >> 1); // critical! don't wait for too long
  return sw_uart_getc_ll(); 
}

// Robust version of get char
// receiving robust_iter times of the same char
// and vote for the correct one
unsigned char sw_uart_getc_robust(unsigned int robust_iter) {
  char hash[256] = {0};
  unsigned char byte = -1;
  for (int i = 0; i < robust_iter; i++) {
      unsigned char c = sw_uart_getc();
      hash[c]++;
      if (byte != -1 && hash[c] * 2 > robust_iter)
          byte = c;
  }
  return byte;
}

// Low-level put char
void sw_uart_putc_ll(char c) {
  gpio_write(TX_SOFT, LOW);
  delay_us(DELAY);
  for (unsigned bit = 0; bit < 8; bit++) {
    gpio_write(TX_SOFT, (c >> bit) & 1);
    delay_us(DELAY);
  }
  gpio_write(TX_SOFT, HIGH);
  delay_us(DELAY);
}

// High-level put char
void sw_uart_putc(char c) {
  sw_uart_putc_ll(c);
}

// Robust version of put char
// sending robust_iter times of the same char
// to let the end side vote for the correct one
void sw_uart_putc_robust(unsigned char c, unsigned int robust_iter) {
  for (int i = 0; i < robust_iter; i++)
    sw_uart_putc(c);
}

// Read data until newline
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

// Send a line of data
void sw_uart_writeline(const char* data) {
  int i = 0;
  while (data[i] != '\n') {
    char c = data[i];
    sw_uart_putc(c);
  }
  sw_uart_putc('\n');
}

// Send nbytes of data
void sw_uart_send_data(const char* data, unsigned nbytes) {
  for (unsigned i = 0; i < nbytes; i++) {
    char c = data[i];
    sw_uart_putc(c);
  }
}

/* INIT */

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
  // TODO: add rxpin, txpin, baudrate as argument
  // instead of letting them be hardcoded
  sw_uart_init_rx();
  sw_uart_init_tx();
}
