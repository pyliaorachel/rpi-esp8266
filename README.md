# Adding networking to the pi with esp8266
## Authors: Peiyu Liao, Stephanie Wang

## Set up
We've connected an ESP8266 WiFi module to our pi, which can be used to talk to servers via WiFi and send over data that has been gathered by sensors, or talk with another ESP8266 module.  
The firmware running on the ESP is NodeMCU, and includes a Lua interpreter. We send Lua programs to run on the module via the ESP's UART.  
Because our pi has only one hardware UART (one set of TX, RX pins), we wrote a software UART choosing two arbitrary GPIO pins as the TX, RX pins. Thus our pi communicates with unix via the software UART, and also communicates with the ESP module via the native UART. To write programs to the ESP, we extended the (lab10-shell)[https://github.com/dddrrreee/cs140e-win19/tree/master/labs/lab10-shell to launch a shell that sends Lua commands to the ESP. Our final demonstration shows that we can use our ESP as a client to communicate with a server, which is useful for projects that require networking.

## Software UART


## ESP Shell
We extended the (lab10-shell)[https://github.com/dddrrreee/cs140e-win19/tree/master/labs/lab10-shell] to include a command that launches a shell that communicates with the ESP. This is how we send programs to the ESP. 

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
