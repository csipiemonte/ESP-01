#include <Arduino.h>
#include <SoftwareSerial.h>
#define MAXBUFSZIE 256

enum STATUSCODE {
  READY,
  INVALIDCMD,
  CMDOK,
  CMDTIMEOUT,
  SSLCONNECTERR,
  SSLGETERR
};

boolean parseCmd (char*  cmd, String* params);

class espSerial {
  private:
    byte txPin;
    byte rxPin;
    byte enPin;
    SoftwareSerial* serial;
    boolean waitForDone();
  public:
    espSerial(byte rxPin, byte txPin, byte enPin);
    boolean setWifi(String sid, String psk);
    boolean sslGet(String& body, String host, unsigned int port, String url, String user, String pwd);
    boolean sslPost(String& body, String host, unsigned int port, String url, String data, String user, String pwd);

};

