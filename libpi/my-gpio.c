/*
 * blink for arbitrary pins.    
 * Implement:
 *	- gpio_set_output;
 *	- gpio_set_on;
 * 	- gpio_set_off.
 *
 *
 * - try deleting volatile.
 * - change makefile to use -O3
 * - get code to work by calling out to a set32 function to set the address.
 * - initialize a structure with all the fields.
 */

// see broadcomm documents for magic addresses.
#define GPIO_BASE 0x20200000
volatile unsigned *gpio_fsel0 = (volatile unsigned *)(GPIO_BASE + 0x00);
volatile unsigned *gpio_set0  = (volatile unsigned *)(GPIO_BASE + 0x1C);
volatile unsigned *gpio_clr0  = (volatile unsigned *)(GPIO_BASE + 0x28);

/* unsigned get32(volatile void *addr) {
    return *(volatile unsigned *)addr;
}

void (put32)(volatile void *addr, unsigned val) {
    *(volatile unsigned *)addr = val;
} */

int gpio_set_function(unsigned pin, unsigned val) {
    if(pin >= 32)
        return -1;
    if((val & 0b111) != val)
        return -1;

    unsigned *gp = ((unsigned *) GPIO_BASE + pin/10);
    unsigned off = (pin % 10) * 3;

    unsigned v = get32(gp);
    v &= ~(0b111 << off);
    v |= (val << off);

    put32(gp, v);
    return 0;
}

// XXX might need memory barriers.
int gpio_set_output(unsigned pin) {
	if(pin >= 32)
		return -1;
    volatile unsigned *addr = (volatile unsigned *) ((int) gpio_fsel0 + (pin / 10 * 4));
    int bit_group = pin % 10;
    put32(addr, (get32(addr) & ~(0b111 << (bit_group * 3))) | 0b001 << (bit_group * 3));

    return 0;
}

int gpio_set_on(unsigned pin) {
    if (pin >= 32)
        return -1;
    put32(gpio_set0, 0b1 << pin);
    return 0;
}
int gpio_set_off(unsigned pin) {
    if (pin >= 32)
        return -1;
    put32(gpio_clr0, 0b1 << pin);
    return 0;
}

// countdown 'ticks' cycles; the asm probably isn't necessary.
void delay(unsigned ticks) {
	while(ticks-- > 0)
		asm("add r1, r1, #0");
}