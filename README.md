# Adding Networking to the Pi with ESP8266

Authors: Peiyu Liao, Stephanie Wang

## Introduction

![ESP8266](https://i.ebayimg.com/images/g/1PUAAOSwhQhY2uZF/s-l300.jpg)

We've connected an ESP8266 WiFi module to our pi, which can be used to talk to servers via WiFi and send over data that has been gathered by sensors, or talk with another ESP8266 module.

The firmware running on the ESP is NodeMCU, and includes a Lua interpreter. We send Lua programs to run on the module via the ESP's UART.

Because our pi has only one hardware UART (one set of TX, RX pins), we implemented a software UART choosing two arbitrary GPIO pins as the TX, RX pins. Thus our pi communicates with unix via the software UART, and also communicates with the ESP module via the native UART.

As a project to demonstrate the ability to communicate between the Pi and the ESP, we extended the [lab10-shell](https://github.com/dddrrreee/cs140e-win19/tree/master/labs/lab10-shell) to launch a shell that sends Lua commands to the ESP. Other applications can take reference from the pi-side shell code to communicate with the ESP.

Our final demonstration shows that we can use our ESP as a client to communicate with a server, which is useful for projects that require networking.

## Setup & Usage

- Hardware
    - Prepare an ESP8266 module with NodeMCU firmware. Here we use the [Adafruit HUZZAH ESP8266 breakout](https://learn.adafruit.com/adafruit-huzzah-esp8266-breakout/overview).
    - Connect your USB-TTL adapter from your laptop to the software UART GPIO pins. In our implementation, GPIO 5 is assigned as TX, and GPIO 6 is assigned as RX. You can change the pin assignments in [sw-uart-libpi/sw-uart.h](https://github.com/pyliaorachel/rpi-esp8266/blob/master/sw-uart-libpi/sw-uart/sw-uart.h).
    - Connect the ESP's dedicated UART pins to the native UART pins on your pi. Connect the power line to either 5V or 3.3V.
- Software UART Bootloader
    - Copy [bootloader/pi-side/kernel.img](https://github.com/pyliaorachel/rpi-esp8266/blob/master/project/bootloader/pi-side/kernel.img) to your SD card as `kernel.img`, as our bootloader is now modified to use the software UART.
- Run the Pi shell
    - Plug in your Pi.
    - Set `LIBPI_PATH` to the [sw-uart-libpi](https://github.com/pyliaorachel/rpi-esp8266/tree/master/sw-uart-libpi) directory.
        - Open `~/.bashrc` and write `export LIBPI_PATH="/path/to/sw-uart-libpi"`.
        - Run `source ~/.bashrc`.
    - Change directory to [project](https://github.com/pyliaorachel/rpi-esp8266/tree/master/project) and run `make` in the project folder.
    - Change directory to [project/shell-unix-side](https://github.com/pyliaorachel/rpi-esp8266/tree/master/project/shell-unix-side) and run `make run`, which will use the software UART to `my-install` the pi-shell program. Now you should see a the pi-shell prompt.
- Run the ESP shell
    - Enter `esp` to open the ESP shell, which tells the Pi to communicate with the ESP.
    - You will be asked to reset the ESP which is necessary to launch the Lua interpreter. Long press on the reset button on the ESP and release. You will see a "Welcome!" message.
    - Enter some Lua commands that will be sent over to the Pi to control the ESP.
        - Try the following commands to turn on the LED light:
            - `gpio.mode(3, gpio.OUTPUT)`
            - `gpio.write(3, gpio.LOW)`
        - The commands will be echoed back to you from the Pi. These do not come from the ESP.
    - Exit the ESP shell by entering `exit`. This takes the unix shell back to the original Pi shell state.

Your terminal should look similar to this:

```bash
$ make run
../bootloader/unix-side/my-install -exec ./pi-shell ../shell-pi-side/pi-shell.bin
my-install: going to clean up UART
my-install: about to boot
Sending program binary......................................Done.
PIX:> esp
PIX:buildin cmd
PIX:esp cmd
ESP:Please reset your ESP8266...
ESP:Welcome!
ESP:> gpio.mode(3, gpio.OUTPUT)
ESP:pi echoed: <gpio.mode(3, gpio.OUTPUT)>
ESP:> gpio.write(3, gpio.LOW)
ESP:pi echoed: <gpio.write(3, gpio.LOW)>
ESP:> exit
ESP:ESP shell exited
PIX:> ^CPIX:
got control-c: going to shutdown pi.
PIX:builtin cmd: reboot
make: *** [run] Interrupt: 2

PIX:PI REBOOT!!!
PIX:pi rebooted.  shell done.
```

## Implementation Details

### Software UART Bootloader

Software UART is a bit banging technique that performs serial communication over two GPIO pins in a low-level way. To free up the native UART port for the ESP, we implemented a software UART and adapt our bootloader and my-install programs to use it. [sw-uart-libpi/sw-uart](https://github.com/pyliaorachel/rpi-esp8266/tree/master/libpi/sw-uart) contains the implementation of the software UART.

> ###### The interrupt codes in the package
> You will see `gpio-int` and `timer-int` in the package, which is the original RX implementation that detects the start signal using GPIO interrupts. We have get rid of the interrupt implementation and used a busy-waiting loop instead. You can ignore these two folders and the `int_handler()` function in `sw-uart.c`. We left it there since it works if you uncomment line 118 in `sw-uart.c` in the `sw_uart_init_rx()` function that enables the interrupts.

###### UART Protocol

![protocol](https://cdn.sparkfun.com/assets/1/8/d/c/1/51142c09ce395f0e7e000002.png)

In the UART protocol, a byte transmitted is preceded with a start bit (low signal) and ends with a stop bit (high signal), hence a total of 10 bits. The line is held high when idle. The time delay between two bits is `1 / baudrate`, e.g. it is 104 μs for a baudrate of `9600`.

The steps for implementing the software TX / RX pins are thus:

- TX pin
    - Sends a low signal (start bit). Delay for 104 μs.
    - Sends the 8 bits of the data byte. Delay each for 104 μs.
    - Sends a high signal (stop bit). Delay for 104 μs.
- RX pin
    - Waits for a low signal (start bit). Delay for 104 μs.
    - Reads 8 bits of data. Delay each for 104 μs.
    - Reads a high signal (stop bit).

###### Robust Communication

Software UART is less reliable than a native UART port. If we are using it to transmit a lot of data, it is better to do some hacks to ensure a zero data corruption rate.

The method we use is to transmit a byte redundantly and let the receiving side vote for the correct byte. We refer to this method as the robust method. There can be other hacks such as increasing the bit sampling rate, which is more ideal since we don't need to change the code at the transmission side and also should be more efficient.

###### Adapting the `bootloader` and `my-install`

After the `putc` and `getc` methods are implemented for the software UART, we can easily switch between the methods of native uart and software uart by e.g. defining a macro in [bootloader/shared-code/simple-boot.h](https://github.com/pyliaorachel/rpi-esp8266/blob/master/project/bootloader/shared-code/simple-boot.h). Now the bootloader and my-install knows they should use the robust method for communication.

Another thing to note is the timout issue of the file descriptor. In our bootloader protocol, `my-install` waits for the `ACK` signal after sending the binary data and the `EOT` signal. If we implement the robust method, the data transmission may take too long, and `my-install` may timeout on waiting for `ACK`.

One solution is to increase the timeout time of the file descriptor in [bootloader/unix-side/my-install.c](https://github.com/pyliaorachel/rpi-esp8266/blob/master/project/bootloader/unix-side/my-install.c). A possibly better solution, which is implemented, is to let the bootloader send back an `ACK` every several bytes of data received. See `get_binary()` function in [bootloader.c](https://github.com/pyliaorachel/rpi-esp8266/blob/master/project/bootloader/pi-side/bootloader.c) and `send_binary()` function in [simple-boot.c](https://github.com/pyliaorachel/rpi-esp8266/blob/master/project/bootloader/unix-side/simple-boot.c).

### ESP Shell

We create an interface connected to the Lua interface of the ESP through the Pi. This way we can send Lua commands to the ESP as if the ESP is directly connected to the unix side. This may not seem useful as a practical application, but it demonstrates that the Pi can talk to and control the ESP through the native UART port, and hence we can say that the Pi is equipped with a network function.

The ESP shell is integrated with the Pi shell from [lab10](https://github.com/dddrrreee/cs140e-win19/tree/master/labs/lab10-shell). It can be opened by sending an `esp` command to the Pi shell and exited by sending an `exit` command.

###### Unix-Side Shell

The `esp()` function in [project/shell-unix-side/pi-shell.c](https://github.com/pyliaorachel/rpi-esp8266/blob/master/project/shell-unix-side/pi-shell.c) handles the ESP shell interface. It is straightforward, just reading the commands from user and send it to Pi, and read the response from the Pi.

Note that the ESP takes commands that end with `\r\n`, i.e. CRLF line endings, hence we need to wrap the commands a bit before passing to Pi.

###### Pi-Side Shell

The `strncmp(buf, "esp", 3) == 0` branch section in `notmain()` of [project/shell-pi-side/pi-shell.c](https://github.com/pyliaorachel/rpi-esp8266/blob/master/project/shell-pi-side/pi-shell.c) opens the communication channel with ESP. It reads the commands from the unix side, sends it to ESP, reads the response from ESP, and sends it back to the unix side.

In order to read the response correctly, it is critical to understand how the Lua interface of the ESP works. We experimented with the ESP directly via [CoolTerm](http://freeware.the-meiers.org/), and found out that every character sent to the ESP will be echoed back. Thus in the function that writes data, make sure every echoed byte is consumed, or else the response parsed will not be as expected. In addition to the echoed bytes, there are some garbage bytes sent from the ESP after reset. Make sure to read them away.

To read the actual response from the ESP after it executes the command, we simply read bytes until the prompt ">" appears.

### Demo: Making Two ESPs Talk to Eachother

We'll send a simple "hello world" from client (our ESP connected to Pi) to a server ESP (setup using CoolTerm). The following scripts are taken from this tutorial on [making two ESP8266 talk](https://randomnerdtutorials.com/how-to-make-two-esp8266-talk/).  

First load the server script below to the server ESP, which creates an access point with the specified ssid and password, and listens for incoming data:

```
print("ESP8266 Server")
wifi.setmode(wifi.STATIONAP);
wifi.ap.config({ssid="test",pwd="12345678"});
print("Server IP Address:",wifi.ap.getip())

sv = net.createServer(net.TCP) 
sv:listen(80, function(conn)
    conn:on("receive", function(conn, receivedData) 
        print("Received Data: " .. receivedData)         
    end) 
    conn:on("sent", function(conn) 
      collectgarbage()
    end)
end)
```

Then send the following commands to the client ESP through the ESP shell, which lets the client connect with the server, and send a "hello world" message to the server:
```
print("ESP8266 Client")
wifi.sta.disconnect()
wifi.setmode(wifi.STATION) 
wifi.sta.config("test","12345678") -- connecting to server
wifi.sta.connect() 

print(wifi.sta.getip()) -- make sure you see the ip address

cl=net.createConnection(net.TCP, 0)
cl:connect(80,"192.168.4.1")
cl:send("Hello World!")
```

You should see "Received Data: Hello World!" appearing on your server side.

## Future Work

- Wrap the communication with ESP into a package and export its interface, instead of scattering the communication details in the pi-shell.
- Implement a tool that loads a lua script file and sends it to ESP.
- Implement a more practical application that sends data to the cloud through the ESP.

## Supporting Documents

1. [The official Adafruit ESP8266 Manual](https://cdn-learn.adafruit.com/downloads/pdf/adafruit-huzzah-esp8266-breakout.pdf?timestamp=1551961408)
2. [BCM2835-ARM-Peripherals documentation](https://github.com/dddrrreee/cs140e-win19/blob/master/docs/BCM2835-ARM-Peripherals.pdf)
