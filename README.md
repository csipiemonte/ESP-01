This project permits to the sensors based on Arduino and ESP-01 to comunicate to the SDP in a secure way through HTTPS.

It's composed by two parts:

**ESP_SERIAL**

It's the firmware for ESP-01 chip. It implements the following functions:
* WPA WIFI association
* HTTPS GET
* HTTPS POST

It communicates with the external world (e.g. Arduino) through the Serial port

**ESP_SERIAL_ARDUINO**

It's a SDK for arduino that implements low level serial commands to ESP-01.

    espSerial(byte rxPin, byte txPin, byte enPin);
    boolean setWifi(String sid, String psk);
    boolean sslGet(String& body, String host, unsigned int port, String url, String user, String pwd);
    boolean sslPost(String& body, String host, unsigned int port, String url, String data, String user, String pwd);

