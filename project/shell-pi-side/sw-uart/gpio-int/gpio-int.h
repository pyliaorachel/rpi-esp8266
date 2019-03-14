#include "rpi.h"
#include "rpi-armtimer.h"
#include "rpi-interrupts.h"

#define RISING_EDGE 0
#define FALLING_EDGE 1

void gpio_int_init(const int pin, const int edge_direction);