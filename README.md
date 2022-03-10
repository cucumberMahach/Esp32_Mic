## Example of using an INMP441 microphone with ESP32 over WiFi
In main.cpp it is necessary to change the values of the pins and WiFi settings to your own.

The _site folder contains the client's page for playing audio over websocket.
In the code, you need to replace the ESP32 address with your own. Put the port on 81. Remove 'audio'.

If you want to use the microphone not only for the address 127.0.0.1, then use my WebsocketService project to create a websocket tunnel with encryption.
To put the same certificate for encryption in this program and on the site page (to accept it on the computer).
Then access will only be over https
