#include "espSerial.h"

boolean parseCmd (char*  cmd, String* params) {
  char* p;
  byte i = 0;
  p = strtok (cmd, ",");
  while (p != NULL)
  {
    params[i++] = p;
    p = strtok (NULL, ",");
  }
  return true;
}

espSerial::espSerial(byte rxPin, byte txPin, byte enPin) {
  pinMode(enPin, OUTPUT);
  digitalWrite(enPin, HIGH);
  serial = new SoftwareSerial(rxPin, txPin);
  serial->begin(19200);
}

boolean espSerial::setWifi(String sid, String psk) {
    
    serial->print((String) "SETWIFI " + sid + "," + psk+"\n");

  if (!waitForDone())
    return false;
}


boolean espSerial::sslGet(String& body, String host, unsigned int port, String url, String user = "", String pwd = "") {
  boolean ret=true;
  serial->print((String) "SSLGET " + host + "," + port + "," + url + "," + user + "," + pwd+"\n");

  if (!waitForDone())
    ret=false;
  body="";
  while (true) {
    String line = serial->readStringUntil('\n');
    if (line==".\r") {
      return ret;
    } else {
      body = body+line+"\n";
    }

  }
  return ret;

}
boolean espSerial::sslPost(String& body, String host, unsigned int port, String url, String data, String user = "", String pwd = "") {
  boolean ret=true;
  serial->print((String) "SSLPOST " + host + "," + port + "," + url + "," + user + "," + pwd+","+data+"\r");
   
  if (!waitForDone())
    ret=false;
  body="";
  while (true) {
    String line = serial->readStringUntil('\n');
    if (line==".\r") {
      return ret;
    } else {
      body = body+line+"\n";
    }

  }
  return ret;

}

boolean espSerial::waitForDone() {
  while (true) {
    String line = serial->readStringUntil('\n');
    Serial.println(line);
    if (line.startsWith("DONE")) {
      String param[3];
      char lineBuf[MAXBUFSZIE];
      // convert C style char array
      line.toCharArray(lineBuf, MAXBUFSZIE);
      ::parseCmd(lineBuf, param);
      
      if (param[1].toInt() != CMDOK) {
        return false;
      } else {
        return true;
      }
    }
  }
}

