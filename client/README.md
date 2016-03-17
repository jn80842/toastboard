code for toastboard device software

To run websocket code:
1. Put SSID and password for your wifi network in the ssid and pass variables.
2. Clone https://github.com/brandenhall/Arduino-Websocket into your Energia libraries folder.
3. Navigate to that repo, open sha1.cpp and delete the line "#include <avr/io.h>". This library doesn't exist for the cc3200, but also isn't used in this repo so we can safely delete it.
4. Build this file to the cc3200. Watch the serial output to see when the wifi has finished connecting.
5. Go to http://www.websocket.org/echo.html and put the IP address of your device in the Location field. Mine is "ws://10.0.1.15"; yours will be printed to serial after connecting to the wifi.
6. Click connect. Send "start" to get (fake) row data once; send "stream" to get row data every 4 seconds; send "stop" to stop the stream.