#include "gpio-int/gpio-int.h"

#define TX_SOFT 5
#define RX_SOFT 6
#define DELAY 104U // baudrate 9600
#define LOW 0
#define HIGH 1

unsigned RX_DATA = 0;

void int_handler(unsigned pc) {
  if (gpio_event_detected(RX_SOFT)) {
    delay_us(DELAY >> 1);

    // Latch 8 bits of data
    for (int i = 0; i < 8; i++) {
      unsigned data = gpio_read(RX_SOFT);
      RX_DATA |= data << i;
      delay_us(DELAY);
    }
    // End bit
    gpio_read(RX_SOFT);

    uart_putc(RX_DATA);
    RX_DATA = 0;

    gpio_event_clear(RX_SOFT);
  }
}

void sw_uart_transmit(char c) {
  gpio_write(TX_SOFT, LOW);
  delay_us(DELAY);
  for (unsigned bit=0; bit < 8; bit++) {
    gpio_write(TX_SOFT, (c >> bit) & 1);
    delay_us(DELAY);
  }
  gpio_write(TX_SOFT, HIGH);
  delay_us(DELAY);
}

void sw_uart_send_data(const char* data, unsigned nbytes) {
  for (unsigned i=0; i< nbytes; i++) {
    char c = data[i];
    sw_uart_transmit(c);
  }
}

void sw_uart_init() {
  gpio_int_init(RX_SOFT, FALLING_EDGE);
  RX_DATA = 0;
}
