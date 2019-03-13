/* 
 * very simple bootloader.  more robust than xmodem.   (that code seems to 
 * have bugs in terms of recovery with inopportune timeouts.)
 */

#define __SIMPLE_IMPL__
#include "../shared-code/simple-boot.h"

#include "libpi.small/rpi.h"

#define ADDR_LIMIT 0x20000000 // Start of peripheral

static void send_byte(unsigned char uc) {
	uart_putc(uc);
}
static unsigned char get_byte(void) { 
    return uart_getc();
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

static void die(int code) {
        put_uint(code);
        reboot();
}

// simple utility function to check that a u32 read matches <v>.
void expect(unsigned v) {
	unsigned x = get_uint();
	if (x != v) {
        die(NAK);
    }
}

// receive nbytes of binary data, put them to base
void get_binary(unsigned * base, unsigned nbytes) {
    unsigned nloaded = 0;
    while (nloaded < nbytes) {
        *base = get_uint();
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
	uart_init();

	// XXX: cs107e has this delay; doesn't seem to be required if 
	// you drain the uart.
	delay_ms(500);

    // Setup: get SOH, nbytes, checksum of program
    unsigned soh = get_uint();
    unsigned n = get_uint();
    unsigned checksum = get_uint();

    // Check if program size too big
    if (ARMBASE + n >= ADDR_LIMIT)
        die(NAK);

    // Send back setup info
    put_uint(soh);
    put_uint(crc32(&n, sizeof(n)));
    put_uint(checksum);

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
    put_uint(ACK);

	// XXX: appears we need these delays or the unix side gets confused.
	// I believe it's b/c the code we call re-initializes the uart; could
	// disable that to make it a bit more clean.
	delay_ms(500);

	// Run what client sent
    BRANCHTO(ARMBASE);

	// Should not get back here, but just in case.
	reboot();
}
