#include "rpi.h"
#include "uart.h"
#include "gpio.h"
#include "mem-barrier.h"

// Refer to BCM2835 ARM Peripherals documentation
// p.8 (replace 0x7E21 with 0x2021)
enum {
    AUX_IRQ = 0x20215000,
    AUX_ENABLES = 0x20215004,
    AUX_MU_IO_REG = 0x20215040,
    AUX_MU_IER_REG = 0x20215044,
    AUX_MU_IIR_REG = 0x20215048,
    AUX_MU_LCR_REG = 0x2021504C,
    AUX_MU_MCR_REG = 0x20215050,
    AUX_MU_LSR_REG = 0x202154,
    AUX_MU_MSR_REG = 0x20215058,
    AUX_MU_SCRATCH = 0x2021505C,
    AUX_MU_CNTL_REG = 0x20215060,
    AUX_MU_STAT_REG = 0x20215064,
    AUX_MU_BAUD_REG = 0x20215068
};

// use this if you need memory barriers.
// void dev_barrier(void) {
// 	dmb();
// 	dsb();
// }

void uart_init(void) {
    // Set GPIO (p.102)
    gpio_set_function(GPIO_PIN14, GPIO_FUNC_ALT5); // TXD1
    gpio_set_function(GPIO_PIN15, GPIO_FUNC_ALT5); // RXD1

    dev_barrier();

    // Enable AUX (p.9, mini UART enable)
    PUT32(AUX_ENABLES, 0b1);

    dev_barrier();

    // Clear TX, RX (p.16-17, transmitter/receiver enable)
    PUT32(AUX_MU_CNTL_REG, 0);

    // Init UART
    PUT32(AUX_MU_IO_REG, 0);            // p.11
    PUT32(AUX_MU_IER_REG, 0);           // p.12
    PUT32(AUX_MU_IIR_REG, 0b11000111);  // p.13, clear receive/transmit FIFO
    PUT32(AUX_MU_LCR_REG, 0b11);        // p.14, data size to 8-bit
    PUT32(AUX_MU_MCR_REG, 0);           // p.14
    PUT32(AUX_MU_LSR_REG, 0b01000000);  // p.15
    PUT32(AUX_MU_MSR_REG, 0b00100000);  // p.15
    //PUT32(AUX_MU_BAUD_REG, 270);      // p.19, baudrate 115200, formula p.11
    PUT32(AUX_MU_BAUD_REG, 3254);       // p.19, baudrate 9600, formula p.11

    // Set TX, RX
    PUT32(AUX_MU_CNTL_REG, 0b11);
    
    dev_barrier();
}

int uart_getc(void) {
    int c;
    while (1) {
        unsigned w = GET32(AUX_MU_STAT_REG); // p.18-19
        if (w & 0b1) { // symbol available
            c = GET32(AUX_MU_IO_REG) & 0b11111111; // p.11, receive data
            break;
        }
    }
	return c;
}
void uart_putc(unsigned c) {
    while (1) {
        unsigned w = GET32(AUX_MU_STAT_REG); // p.18-19
        if (w & 0b10) { // space available
            PUT32(AUX_MU_IO_REG, c & 0xFF); // p.11, transmit data
            break;
        }
    }
}
