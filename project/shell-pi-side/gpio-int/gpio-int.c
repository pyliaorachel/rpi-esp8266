/* make GPIO pin generate an interrupt when it goes low to high.  */
#include "gpio-int.h"

// client has to define this.
void int_handler(unsigned pc) {
  // detect (1) if the gpio event triggered and (2) clear it.
  printk("!!!! In the handler!!!\n");
  if (gpio_event_detected(pin)) {
    printk("!!!Detected!!!!\n");
    gpio_event_clear(pin);
  }
}

void OR_IN32(unsigned addr, unsigned val) {
  PUT32(addr, GET32(addr) | val);
}

int gpio_event_detected(unsigned pin) {
  if(pin >= 32)
    return -1;
  if (GET32(GPEDS0) & (1 << pin))
    return 1;
  return 0;
}

int gpio_event_clear(unsigned pin) {
  if(pin >= 32)
    return -1;
  PUT32(GPEDS0, 1 << pin);
  return 0;
}

int gpio_int_rising_edge(unsigned pin) {
  if(pin >= 32)
    return -1;
  PUT32(GPAREN0, 1 << pin);
  return 0;
}

void gpio_int_init(const int pin) {
  printk("about to install handlers\n");
  install_int_handlers();
  
  dev_barrier();
  
  // BCM2835 manual, section 7.5
  PUT32(INTERRUPT_DISABLE_1, 0xffffffff);
  PUT32(INTERRUPT_DISABLE_2, 0xffffffff);
  
  dev_barrier();
  
  // Bit 52 in IRQ registers enables/disables all GPIO interrupts                                                         
  // Bit 52 is in the second register, so subtract 32 for index                                                              
  PUT32(INTERRUPT_ENABLE_2, (1 << (52 - 32)));
  dev_barrier();
  
  
  printk("setting up GPIO interrupts for pin: %d\n", pin);
  gpio_set_input(pin);
  gpio_set_pulldown(pin);
  
  
  gpio_int_rising_edge(pin);
  // gpio_int_high(pin);
  
  printk("gonna enable ints globally!\n");
  
  
  system_enable_interrupts();
  printk("enabled!\n");
}

/*
void notmain() {
  const int pin = 20;
  uart_init();
  
  printk("about to install handlers\n");
  install_int_handlers();
  
  dev_barrier();
  
  // BCM2835 manual, section 7.5
  PUT32(INTERRUPT_DISABLE_1, 0xffffffff);
  PUT32(INTERRUPT_DISABLE_2, 0xffffffff);
  
  dev_barrier();
  
  // Bit 52 in IRQ registers enables/disables all GPIO interrupts                                                         
  // Bit 52 is in the second register, so subtract 32 for index                                                              
  PUT32(INTERRUPT_ENABLE_2, (1 << (52 - 32)));
  dev_barrier();
  
  
  printk("setting up GPIO interrupts for pin: %d\n", pin);
  gpio_set_input(pin);
  gpio_set_pulldown(pin);
  
  
  gpio_int_rising_edge(pin);
  // gpio_int_high(pin);
  
  printk("gonna enable ints globally!\n");
  
  
  system_enable_interrupts();
  printk("enabled!\n");
  int n = 0;
  while(n < 1000000) {
    delay_ms(1);
  }
  
  clean_reboot();
}
*/
