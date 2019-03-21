# Adding networking to the pi with esp8266
Authors: Peiyu Liao, Stephanie Wang

## Introduction
We've connected an ESP8266 WiFi module to our pi, which can be used to talk to servers via WiFi and send over data that has been gathered by sensors, or talk with another ESP8266 module.  
The firmware running on the ESP is NodeMCU, and includes a Lua interpreter. We send Lua programs to run on the module via the ESP's UART.  
Because our pi has only one hardware UART (one set of TX, RX pins), we wrote a software UART choosing two arbitrary GPIO pins as the TX, RX pins. Thus our pi communicates with unix via the software UART, and also communicates with the ESP module via the native UART. To write programs to the ESP, we extended the [lab10-shell](https://github.com/dddrrreee/cs140e-win19/tree/master/labs/lab10-shell) to launch a shell that sends Lua commands to the ESP. Our final demonstration shows that we can use our ESP as a client to communicate with a server, which is useful for projects that require networking.  

## Setup
To setup the hardware, connect your CP2102 USB-TTL adapter to the software UART GPIO pins. Now connect the ESP's dedicated UART pins to the native UART TX RX pins on your pi. Copy `project/bootloader/pi-side/kernel.img` to your SD card as `kernel.img`, as our bootloader is now modified to use the software UART. Run `make` in the project folder. In `project/shell-unix-side` run `make.run`, which will use the software UART to `my-install` the pi-shell. Now you should see a the pi-shell prompt. Type `esp` and enter, and you will be asked to reset the ESP which is necessary to launch the Lua interpreter. Hit the reset button on the module --  now you can type in Lua commands that will run on the ESP!

## Software UART
In [libpi/sw-uart](https://github.com/pyliaorachel/rpi-esp8266/tree/master/libpi/sw-uart) we define the software UART. We use a robust method of put/get since data transmission with GPIO pins may be unreliable and lose bits. 

## ESP Shell
The pi shell in `/project/shell-pi-side/pi-shell.c` includes a command that launches a prompt to send programs to the ESP. In writing and reading to/from the ESP, we make sure to have line endings set to CRLF "\r\n". Useful functions to point out are:  
`int esp_read_CRLF_line(char *buf, int sz)` reads characters from the ESP until we hit `\r\n`.  
`void esp_write_CRLF_line(char *buf, int nbytes)` writes characters to the ESP with a `\r\n`.  


## Making two ESPs talk to eachother
We'll send a simple "hello world" from client (our ESP) to a server ESP. This code was taken from this tutorial on [making two ESP8266 talk](https://randomnerdtutorials.com/how-to-make-two-esp8266-talk/).  
Load the server script below to the ESP not connected to the pi:
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

Copy the client script below into the ESP shell to send a "hello world" message:
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

## Supporting Documents

1. The official Adafruit ESP8266 Manual: https://cdn-learn.adafruit.com/downloads/pdf/adafruit-huzzah-esp8266-breakout.pdf?timestamp=1551961408
2. BCM2835-ARM-Peripherals documentation: https://github.com/dddrrreee/cs140e-win19/blob/master/docs/BCM2835-ARM-Peripherals.pdf
