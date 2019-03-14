#include "gpio-int/gpio-int.h"

#define TX_SOFT 5
#define RX_SOFT 6
#define DELAY 104U // baudrate 9600

unsigned char RX_DATA = 0;

void int_handler(unsigned pc) {
  if (gpio_event_detected(RX_SOFT)) {
    sw_uart_receive_byte();
  }
}

void sw_uart_receive_byte() {
  delay_us(DELAY);

  // Latch 8 bits of data
  for (int i = 0; i < 8; i++) {
    unsigned data = gpio_read(RX_SOFT);
    RX_DATA |= data << i;
    delay_us(DELAY);
  }
  // End bit
  gpio_read(RX_SOFT);

  printk("%c", RX_DATA);
  RX_DATA = 0;

  gpio_event_clear(RX_SOFT);
}

void sw_uart_send_byte(char c) {
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
  printk(data);
  for (unsigned i = 0; i < nbytes; i++) {
    char c = data[i];
    sw_uart_send_byte(c);
  }
}

void sw_uart_init_rx() {
  gpio_int_init(RX_SOFT, FALLING_EDGE);
  RX_DATA = 0;
}

void sw_uart_init_tx() {
  gpio_set_output(TX_SOFT);
  gpio_write(TX_SOFT, HIGH);
}

void sw_uart_init() {
  sw_uart_init_rx();
  sw_uart_init_tx();
}
