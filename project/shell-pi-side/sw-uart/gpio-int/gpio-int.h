#include "rpi.h"
#include "rpi-armtimer.h"
#include "rpi-interrupts.h"

#define RISING_EDGE 0
#define FALLING_EDGE 1
#define HIGH 1
#define LOW 0

void gpio_int_init(const int pin, const int edge_direction);