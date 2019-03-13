/* simplified bootloader.  */
#include "rpi.h"
#include "pi-shell.h"

#define __SIMPLE_IMPL__
#include "../bootloader/shared-code/simple-boot.h"

static void send_byte(unsigned char uc) {
	uart_putc(uc);
}
static unsigned char get_byte(void) { 
    return uart_getc();
}

unsigned get_uint(void) {
	unsigned u = get_byte();
        u |= get_byte() << 8;
        u |= get_byte() << 16;
        u |= get_byte() << 24;
	return u;
}
void put_uint(unsigned u) {
        send_byte((u >> 0)  & 0xff);
        send_byte((u >> 8)  & 0xff);
        send_byte((u >> 16) & 0xff);
        send_byte((u >> 24) & 0xff);
}

static void die(unsigned err) {
	put_uint(err);
	delay_ms(100); 	// let the output queue drain.
	rpi_reboot();
}

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

// load_code:
//	1. figure out if the requested address range is available.
//	2. copy code to that region.
//	3. return address of first executable instruction: note we have
//	a 8-byte header!  (see ../hello-fixed/memmap)
int load_code(void) {
	unsigned addr = 0;

	// let unix know we are ready.
	put_uint(ACK);

	// bootloader code.
    // Setup: get version, addr, nbytes, checksum of program
    unsigned version = get_uint();
    addr = get_uint();
    unsigned nbytes = get_uint();
    unsigned checksum = get_uint();

    // Check if program size too big
    if (addr < LAST_USED_ADDRESSES || addr + nbytes >= MAX_ADDRESS)
        die(TOO_BIG);

    // Send back ACK
    put_uint(ACK);

    // Get program binary code
    get_binary((unsigned *) addr, nbytes);

    // Get EOT
    expect(EOT);

    // Verify checksum; send ACK or error
    unsigned new_checksum = crc32((unsigned char *) addr, nbytes);
    if (checksum != new_checksum)
        die(BAD_CKSUM);
    put_uint(ACK);

    // give time to flush out; ugly.   implement `uart_flush()`
	delay_ms(100);  

	/* return address */
    return addr + 8;
}
