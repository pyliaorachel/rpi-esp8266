#include "gpio-int/gpio-int.h"

#define TX_SOFT 5
#define RX_SOFT 6
#define DELAY 104U // baudrate 9600

void int_handler(unsigned pc) {
  if (gpio_event_detected(RX_SOFT)) {
    unsigned char data = sw_uart_read_byte();
  }
}

unsigned char sw_uart_getc() {
  while(gpio_read(RX_SOFT) != LOW)
    delay_us(DELAY);
  return sw_uart_read_byte(); 
}

unsigned char sw_uart_read_byte() {
  unsigned char rx_data = 0;
  delay_us(DELAY);

  // Latch 8 bits of data
  for (int i = 0; i < 8; i++) {
    unsigned data = gpio_read(RX_SOFT);
    rx_data |= data << i;
    delay_us(DELAY);
  }
  // End bit
  gpio_read(RX_SOFT);

  gpio_event_clear(RX_SOFT);
  return rx_data;
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

void sw_uart_send_data(const char* data, unsigned nbytes) {
  for (unsigned i = 0; i < nbytes; i++) {
    char c = data[i];
    sw_uart_putc(c);
  }
}

void sw_uart_init_rx() {
  gpio_set_input(RX_SOFT);
  //gpio_int_init(RX_SOFT, FALLING_EDGE);
}

void sw_uart_init_tx() {
  gpio_set_output(TX_SOFT);
  gpio_write(TX_SOFT, HIGH);
}

void sw_uart_init() {
  sw_uart_init_rx();
  sw_uart_init_tx();
}
