/* 
 * very simple bootloader.  more robust than xmodem.   (that code seems to 
 * have bugs in terms of recovery with inopportune timeouts.)
 */

#define __SIMPLE_IMPL__
#include "../shared-code/simple-boot.h"
#include "rpi.h"

#define ADDR_LIMIT 0x20000000 // Start of peripheral

static void send_byte(unsigned char uc) {
	sw_uart_putc(uc);
}
static unsigned char get_byte(void) { 
    return sw_uart_getc();
}

static unsigned get_uint(void) {
	unsigned u = get_byte();
        u |= get_byte() << 8;
        u |= get_byte() << 16;
        u |= get_byte() << 24;
	return u;
}
static void put_uint(unsigned u) {
    send_byte((u >> 0)  & 0xff);
    send_byte((u >> 8)  & 0xff);
    send_byte((u >> 16) & 0xff);
    send_byte((u >> 24) & 0xff);
}

static void send_byte_robust(unsigned char uc) {
    for (int i = 0; i < ROBUST_ITER; i++)
	    sw_uart_putc(uc);
}
static unsigned char get_byte_robust(void) {
    char hash[256] = {0};
    unsigned char byte = -1;
    for (int i = 0; i < ROBUST_ITER; i++) {
        unsigned char c = sw_uart_getc();
        hash[c]++;
        if (byte != -1 && hash[c] * 2 > ROBUST_ITER)
            byte = c;
    }
    return byte;
}

static unsigned get_uint_robust(void) {
	unsigned u = get_byte_robust();
        u |= get_byte_robust() << 8;
        u |= get_byte_robust() << 16;
        u |= get_byte_robust() << 24;
	return u;
}
static void put_uint_robust(unsigned u) {
    send_byte_robust((u >> 0)  & 0xff);
    send_byte_robust((u >> 8)  & 0xff);
    send_byte_robust((u >> 16) & 0xff);
    send_byte_robust((u >> 24) & 0xff);
}

static void die(int code) {
    put_uint_robust(code);
    clean_reboot();
}

// simple utility function to check that a u32 read matches <v>.
void expect(unsigned v) {
	unsigned x = get_uint_robust();
	if (x != v) {
        die(NAK);
    }
}

// receive nbytes of binary data, put them to base
void get_binary(unsigned * base, unsigned nbytes) {
    unsigned nloaded = 0;
    while (nloaded < nbytes) {
        *base = get_uint_robust();
        base++;
        nloaded += sizeof(unsigned);
    }
}

//  bootloader:
//	1. wait for SOH, size, cksum from unix side.
//	2. echo SOH, checksum(size), cksum back.
// 	3. wait for ACK.
//	4. read the bytes, one at a time, copy them to ARMBASE.
//	5. verify checksum.
//	6. send ACK back.
//	7. wait 500ms 
//	8. jump to ARMBASE.
//
void notmain(void) {
    sw_uart_init();
	// XXX: cs107e has this delay; doesn't seem to be required if 
	// you drain the uart.
    delay_ms(500);

    // Setup: get SOH, nbytes, checksum of program
    unsigned soh = get_uint_robust();
    unsigned n = get_uint_robust();
    unsigned checksum = get_uint_robust();

    // Check if program size too big
    if (ARMBASE + n >= ADDR_LIMIT)
        die(NAK);

    // Send back setup info
    put_uint_robust(soh);
    put_uint_robust(crc32(&n, sizeof(n)));
    put_uint_robust(checksum);

    // Get ACK
    expect(ACK);

    // Get program binary code
    get_binary((unsigned *) ARMBASE, n);

    // Get EOT
    expect(EOT);

    // Verify checksum; send ACK or error
    unsigned new_checksum = crc32((unsigned char *) ARMBASE, n);
    if (checksum != new_checksum)
    die(NAK);
    put_uint_robust(ACK);
    
    // XXX: appears we need these delays or the unix side gets confused.
    // I believe it's b/c the code we call re-initializes the uart; could
    // disable that to make it a bit more clean.
    delay_ms(500);
    
    // Run what client sent
    BRANCHTO(ARMBASE);
    
    // Should not get back here, but just in case.
    clean_reboot();
}
