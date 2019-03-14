#include "gpio-int/gpio-int.h"

#define TX_SOFT 5
#define RX_SOFT 6
#define DELAY 104U // baudrate 9600

unsigned char RX_DATA = 0;

void int_handler(unsigned pc) {
  if (gpio_event_detected(RX_SOFT)) {
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
}

void sw_uart_init() {
  gpio_int_init(RX_SOFT, FALLING_EDGE);
  RX_DATA = 0;
}