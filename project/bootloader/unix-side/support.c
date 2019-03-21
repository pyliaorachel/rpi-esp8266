#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "demand.h"
#include "../shared-code/simple-boot.h"
#include "support.h"

#define roundup(x,n) (((x)+((n)-1))&(~((n)-1)))

// read entire file into buffer.  return it, write totat bytes to <size>
unsigned char *read_file(int *size, const char *name) {
    // Get file stat
    struct stat fileStat;
    stat(name, &fileStat);

    // Pad filesize
    *size = roundup(fileStat.st_size, sizeof(int));

    // Read file data into buffer
    unsigned char *buf = calloc(*size / sizeof(int), sizeof(int));
    int fildes = open(name, O_RDONLY);
    read(fildes, buf, fileStat.st_size);

    return buf;
}

#define _SVID_SOURCE
#include <dirent.h>
const char *ttyusb_prefixes[] = {
	"ttyUSB",	// linux
	"cu.SLAB_USB", // mac os
	0
};

// filter function to find the exact entry
int filter(const struct dirent *entry) {
    int i = 0;
    // Try to match the entry name with the USB port prefixes
    while (ttyusb_prefixes[i] != NULL) {
        if (strncmp(entry->d_name, ttyusb_prefixes[i], strlen(ttyusb_prefixes[i])) == 0) // port name matched
            return 1;
        i++;
    }
    return 0;
}

// open the TTY-usb device:
//	- use <scandir> to find a device with a prefix given by ttyusb_prefixes
//	- returns an open fd to it
// 	- write the absolute path into <pathname> if it wasn't already
//	  given.
int open_tty(const char **portname) {
    int fildes = 0;

    // Scan directory for portname
    struct dirent **namelist;
    int n = scandir("/dev", &namelist, filter, alphasort);

    if (n == 1) { // unique port found
        // Parse exact path to port
        char pathname[256];
        sprintf(pathname, "/dev/%s", namelist[0]->d_name);
        *portname = strdup(pathname); // deep copy the string into *portname, or the memory got freed after return

        // Get file descriptor of port
        fildes = open(*portname, O_RDWR | O_NOCTTY | O_SYNC);
        if (fildes < 0)
            panic("port not opened");
    } else {
        panic("port scan not equal to 1");
    }
    return fildes;
}
