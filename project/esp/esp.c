#include "rpi.h"

int notmain ( void ) {
	int led1 = 20;
	int led2 = 21;

  	gpio_set_output(led1);
  	gpio_set_output(led2);
        while(1) {
            gpio_set_off(led2);
            gpio_set_on(led1);
            delay_ms(1000);
            gpio_set_on(led2);
            gpio_set_off(led1);
            delay_ms(1000);
        }
	return 0;
}
