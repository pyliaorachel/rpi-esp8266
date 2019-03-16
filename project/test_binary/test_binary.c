#include "rpi.h"

void notmain() {
#if 1
  // Program to write character through sw uart does not work.
  //sw_uart_init();
  //delay_ms(500);
  //sw_uart_putc('h');
  clean_reboot();
#endif
#if 0
  // Blink led program.
  int led = 21;
  gpio_set_output(led);
  int i;
  for (i=0; i < 10; i++) {
    gpio_set_on(led);
    delay(1000000);
    gpio_set_off(led);
    delay(1000000);
  }
  return;
#endif

#if 0
  // Send command to ESP.
  uart_init();
  delay_ms(500);
  char command[] = "gpio.mode(3, gpio.OUTPUT)\r\ngpio.write(3, gpio.LOW)\r\n";
  //char command[] = "gpio.mode(3, gpio.OUTPUT)\r\nwhile 1 do\r\ngpio.write(3, gpio.HIGH)\r\ntmr.delay(100000)\r\ngpio.write(3, gpio.LOW)\r\ntmr.delay(100000)\r\nend\r\n";
  size_t nbytes = sizeof command;
  for (size_t i=0; i < nbytes; i++) {
    uart_putc(command[i]);
  }
  delay_us(100);
  return;
#endif
}

