#include "rpi.h"

void notmain() { 
  
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
}
