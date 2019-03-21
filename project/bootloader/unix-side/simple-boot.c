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

// Choose the uart functions to use
#define SEND_BYTE(fd, c) (IS_SW_UART ? send_byte_robust(fd, c) : send_byte(fd, c))
#define GET_BYTE(fd) (IS_SW_UART ? get_byte_robust(fd) : get_byte(fd))

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

// Send redundante bytes for the receiving side to vote for the correct one.
static void send_byte_robust(int fd, unsigned char b) {
    for (int i = 0; i < ROBUST_ITER; i++) {
        send_byte(fd, b);
    }
}

// Receive redundant bytes and vote for the correct one.
static unsigned char get_byte_robust(int fd) {
    char hash[256] = {0};
    unsigned char byte;
    unsigned char c;
    for (int i = 0; i < ROBUST_ITER; i++) {
        c = get_byte(fd);
        hash[c]++;
        if (hash[c] * 2 > ROBUST_ITER)
            byte = c;
    }
    return byte;
}

unsigned get_uint(int fd) {
    unsigned u = GET_BYTE(fd);
    u |= GET_BYTE(fd) << 8;
    u |= GET_BYTE(fd) << 16;
    u |= GET_BYTE(fd) << 24;
    trace_read32(u);
    return u;
}

void put_uint(int fd, unsigned u) {
    // mask not necessary.
    SEND_BYTE(fd, (u >> 0)  & 0xff);
    SEND_BYTE(fd, (u >> 8)  & 0xff);
    SEND_BYTE(fd, (u >> 16) & 0xff);
    SEND_BYTE(fd, (u >> 24) & 0xff);
    trace_write32(u);
}

// simple utility function to check that a u32 read from the 
// file descriptor matches <v>.
void expect(const char *msg, int fd, unsigned v) {
    unsigned x = get_uint(fd);
	if(x != v) {
        put_uint(fd, NAK);
		panic("%s: expected %x, got %x\n", msg, v, x);
    }
}

// send nbytes of binary data from buf to fd
void send_binary(int fd, unsigned * buf, unsigned nbytes) {
    while (nbytes > 0) {
        put_uint(fd, *buf++);
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
// read/write using put_uint() get_uint().
void simple_boot(int fd, const unsigned char * buf, unsigned n) {
    // Check program should be padded, and n should be multiple of 4
    if (n % 4 != 0)
        panic("nbytes %d not multiple of 4", n);

    // Setup: send SOH, nbytes, checksum of program
    unsigned checksum = crc32(buf, n);

    put_uint(fd, SOH);
    put_uint(fd, n);
    put_uint(fd, checksum);
    expect("SOH error", fd, SOH);
    expect("nbytes error", fd, crc32(&n, sizeof(n)));
    expect("checksum error", fd, checksum);

    // Send ACK
    put_uint(fd, ACK);

    // Send program binary
    fprintf(stderr, "Sending program binary");
    send_binary(fd, (unsigned *) buf, n);
    fprintf(stderr, "Done.\n");

    // Send EOT
    put_uint(fd, EOT);

    // Get ACK
    expect("ACK error", fd, ACK);
}
