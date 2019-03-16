#include <assert.h>
#include <fcntl.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include "demand.h"
#include "trace.h"

#define __SIMPLE_IMPL__
#include "../shared-code/simple-boot.h"

static void send_byte(int fd, unsigned char b) {
	if(write(fd, &b, 1) < 0)
		panic("write failed in send_byte\n");
}
static unsigned char get_byte(int fd) {
	unsigned char b;
	int n;
	if((n = read(fd, &b, 1)) != 1)
		panic("read failed in get_byte: expected 1 byte, got %d\n",n);
	return b;
}

unsigned get_uint(int fd) {
    unsigned u = get_byte(fd);
    u |= get_byte(fd) << 8;
    u |= get_byte(fd) << 16;
    u |= get_byte(fd) << 24;
    trace_read32(u);
    return u;
}

void put_uint(int fd, unsigned u) {
    // mask not necessary.
    send_byte(fd, (u >> 0)  & 0xff);
    send_byte(fd, (u >> 8)  & 0xff);
    send_byte(fd, (u >> 16) & 0xff);
    send_byte(fd, (u >> 24) & 0xff);
    trace_write32(u);
}

static void send_byte_robust(int fd, unsigned char b) {
    for (int i = 0; i < ROBUST_ITER; i++) {
        if (write(fd, &b, 1) < 0)
            panic("write failed in send_byte\n");
    }
}
static unsigned char get_byte_robust(int fd) {
    char hash[256] = {0};
    unsigned char byte;
    unsigned char c;
    int n;
    for (int i = 0; i < ROBUST_ITER; i++) {
        if ((n = read(fd, &c, 1)) != 1)
            panic("read failed in get_byte: expected 1 byte, got %d\n", n);
        hash[c]++;
        if (hash[c] * 2 > ROBUST_ITER)
            byte = c;
    }
    return byte;
}

unsigned get_uint_robust(int fd) {
    unsigned u = get_byte_robust(fd);
    u |= get_byte_robust(fd) << 8;
    u |= get_byte_robust(fd) << 16;
    u |= get_byte_robust(fd) << 24;
    trace_read32(u);
    return u;
}

void put_uint_robust(int fd, unsigned u) {
    // mask not necessary.
    send_byte_robust(fd, (u >> 0)  & 0xff);
    send_byte_robust(fd, (u >> 8)  & 0xff);
    send_byte_robust(fd, (u >> 16) & 0xff);
    send_byte_robust(fd, (u >> 24) & 0xff);
    trace_write32(u);
}

// simple utility function to check that a u32 read from the 
// file descriptor matches <v>.
void expect(const char *msg, int fd, unsigned v) {
    unsigned x = get_uint_robust(fd);
	if(x != v) {
        put_uint_robust(fd, NAK);
		panic("%s: expected %x, got %x\n", msg, v, x);
    }
}

// send nbytes of binary data from buf to fd
void send_binary(int fd, unsigned * buf, unsigned nbytes) {
    while (nbytes > 0) {
        put_uint_robust(fd, *buf++);
        nbytes -= sizeof(unsigned);
        // check every 2^8 bytes the program have been sent correctly
        // this avoids timeout a bit
        if ((nbytes & 0xff) == 0) {
            expect("ACK failed during send binary", fd, ACK);
            fprintf(stderr, "."); // print progress
        }
    }
}

// unix-side bootloader: send the bytes, using the protocol.
// read/write using put_uint_robust() get_uint_robust().
void simple_boot(int fd, const unsigned char * buf, unsigned n) {
    // Check program should be padded, and n should be multiple of 4
    if (n % 4 != 0)
        panic("nbytes %d not multiple of 4", n);

    // Setup: send SOH, nbytes, checksum of program
    unsigned checksum = crc32(buf, n);

    put_uint_robust(fd, SOH);
    put_uint_robust(fd, n);
    put_uint_robust(fd, checksum);
    expect("SOH error", fd, SOH);
    expect("nbytes error", fd, crc32(&n, sizeof(n)));
    expect("checksum error", fd, checksum);

    // Send ACK
    put_uint_robust(fd, ACK);

    // Send program binary
    fprintf(stderr, "Sending program binary");
    send_binary(fd, (unsigned *) buf, n);
    fprintf(stderr, "Done.\n");

    // Send EOT
    put_uint_robust(fd, EOT);
    // Get ACK
    expect("ACK error", fd, ACK);
}
