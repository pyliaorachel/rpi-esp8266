LIBPI = $(LIBPI_PATH)

CC= arm-none-eabi-gcc
CFLAGS = -I$(LIBPI)/.. -I. -Wall -Og -nostdlib -nostartfiles -ffreestanding 

ifndef LIBPI_PATH
$(error LIBPI_PATH is not set: this should contain the absolute path to where this directory is.  Define it in your shell's initialiation.  For example, .tcshrc for tcsh or .bashrc for bash)
endif

# useful set of defines.
include $(LIBPI_PATH)/includes.mk

SRC = bootloader.c 
OBJS = $(SRC:.c=.o)

all : kernel.img

start.o : start.s
	arm-none-eabi-as start.s -o start.o

kernel.img : memmap start.o $(OBJS)
	make -C $(LIBPI)
	arm-none-eabi-ld start.o $(OBJS) $(LIBPI)/libpi.a -T memmap -o bootloader.elf
	arm-none-eabi-objdump -D bootloader.elf > bootloader.list
	arm-none-eabi-objcopy bootloader.elf -O binary kernel.img

clean :
	rm -f *.o *.bin *.elf *.list *.img *~ Makefile.bak
	make -C $(LIBPI) clean

depend:
	makedepend -I$(LIBPI)/.. *.[ch] 

# DO NOT DELETE
