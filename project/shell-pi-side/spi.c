#include "spi.h"
#include "rpi.h"

void spi_data(char* data, int nbytes) {
    for (int i = 0; i < nbytes; i++)
        spi_one_byte(data[i]);
}

void spi_one_byte(unsigned int x) {
    // p.155, set transfer active TA = 1 & clear FIFO
    PUT32(AUX_SPI0_CS,0x000000B0);

    // Wait for FIFO can accept data
    while (1)
        // p.154, TXD TX FIFO can accept data
        if (GET32(AUX_SPI0_CS) & (1<<18))
            break;

    // p.156, send to FIFO
    PUT32(AUX_SPI0_FIFO,x&0xFF);

    // Wait for transfer to be done
    while (1)
        // p.154, Done transfer
        if (GET32(AUX_SPI0_CS) & (1<<16))
            break;

    // p.155, set transfer active TA = 0, i.e. pull CS line high
    PUT32(AUX_SPI0_CS,0x00000000);
}

void spi_init(void) {
    unsigned int ra;

    // p.9 anable spi0
    ra=GET32(AUX_ENABLES);
    ra|=2;
    PUT32(AUX_ENABLES,ra);

    // p.102 select GPIO pin functions
    ra=GET32(GPFSEL0);
    ra&=~(7<<27);   //gpio9
    ra|=4<<27;      //alt0
    ra&=~(7<<24);   //gpio8
    ra|=4<<24;      //alt0
    ra&=~(7<<21);   //gpio7
    ra|=4<<21;      //alt0
    PUT32(GPFSEL0,ra);
    ra=GET32(GPFSEL1);
    ra&=~(7<<0);    //gpio10/
    ra|=4<<0;       //alt0
    ra&=~(7<<3);    //gpio11/
    ra|=4<<3;       //alt0
    PUT32(GPFSEL1,ra);

    PUT32(AUX_SPI0_CS,0x0000030);   // p.155 clear TX/RX FIFO
    PUT32(AUX_SPI0_CLK,0x0000);     // p.156, clock rate, slowest possible, could probably go faster here
}

//-------------------------------------------------------------------------
//
// Copyright (c) 2014 David Welch dwelch@dwelch.com
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//-------------------------------------------------------------------------
