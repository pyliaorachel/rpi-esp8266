// modified cs107e so we can do error checking.
#ifndef __GPIO_H__
#define __GPIO_H__

// GPIO pin mappings for UART
#define GPIO_TX 14
#define GPIO_RX 15

#define GPIO_BASE 0x20200000
#define GPSET0  (GPIO_BASE+0x1c)
#define GPCLR0  (GPIO_BASE+0x28)
#define GPLEV0  (GPIO_BASE+0x34)
// GPIO Pin Rising Edge Detect Enable 0/1                                                                                              
#define GPREN0 (GPIO_BASE+0x4C)
#define GPREN1 (GPIO_BASE+0x50)
// GPIO Pin Falling Edge Detect Enable 0/1                                                                                             
#define GPFEN0 (GPIO_BASE+0x58)
#define GPFEN1 (GPIO_BASE+0x5C)
// GPIO Pin Async Rising Edge Detect Enable 0/1                                                                                        
#define GPAREN0 (GPIO_BASE+0x7C)
#define GPAREN1 (GPIO_BASE+0x80)
// GPIO Pin Async Falling Edge Detect Enable 0/1                                                                                       
#define GPAFEN0 (GPIO_BASE+0x88)
#define GPAFEN1 (GPIO_BASE+0x8C)
// GPIO Pin Event Detect Status 0/1                                                                                                    
#define GPEDS0 (GPIO_BASE+0x40)
#define GPEDS1 (GPIO_BASE+0x44)
enum {
    GPIO_FUNC_INPUT   = 0,
    GPIO_FUNC_OUTPUT  = 1,
    GPIO_FUNC_ALT0    = 4,
    GPIO_FUNC_ALT1    = 5,
    GPIO_FUNC_ALT2    = 6,
    GPIO_FUNC_ALT3    = 7,
    GPIO_FUNC_ALT4    = 3,
    GPIO_FUNC_ALT5    = 2,
};

/*
 * Set a GPIO function for GPIO pin number `pin`. Settings for other pins
 * should be unchanged.  -1 on error.
 */
int gpio_set_function(unsigned pin, unsigned function);

/*
 * Get the GPIO function for GPIO pin number `pin`.  -1 on error.
 */
unsigned gpio_get_function(unsigned pin);

/* Convenience functions for setting a pin to input or output.  */
int gpio_set_input(unsigned pin);
int gpio_set_output(unsigned pin);

/*
 * Set GPIO pin number `pin` to high (1) or low (0). This
 * function assumes the pin is already in output mode.
 * Settings for other pins should be unchanged.
 * -1 on error.
 */
int gpio_write(unsigned pin, unsigned val);

/*
 * Get current level (1 for high, 0 for low) for GPIO pin number `pin`.
 *
 * @param pin the GPIO pin number to read the value of
 * @return    the value of the specified pin
 */
unsigned gpio_read(unsigned pin);

enum {
    GPIO_PUD_DISABLE  = 0,
    GPIO_PUD_PULLDOWN = 1,
    GPIO_PUD_PULLUP   = 2,
};

int gpio_set_pud(unsigned pin, unsigned pud);
int gpio_set_pullup(unsigned pin);
int gpio_set_pulldown(unsigned pin);

int gpio_set_off(unsigned pin);
int gpio_set_on(unsigned pin);
#endif
