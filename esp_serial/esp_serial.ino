#define DEBUG_HTTPCLIENT(...) Serial.printf( __VA_ARGS__ )
#include <ESP8266WiFi.h>
//#include <WiFiClientSecure.h>
#include <ESP8266httpClient.h>

#include "b64.h"

#define MAXCMDSZIE 256
#define MAXARGSIZE 128
#define MAXPARAMS 10
#define TIMEOUT 10
#define _DEBUG_



enum STATUSCODE {
  READY,
  INVALIDCMD,
  CMDOK,
  CMDTIMEOUT,
  SSLCONNECTERR,
  SSLHTTPERR
};

char statusmsg[][128] = {
  "Ready",
  "Invalid command",
  "Ok",
  "Timeout",
  "Error connecting to SSL host",
  "HTTP Error"
};

void sendStatus(byte s) {
  String msg = (String) "DONE," + s + "," + statusmsg[s];
  Serial.println(msg);
}

void sendStartCmd() {
  String msg = (String) "START";
  Serial.println(msg);
}

void sendProgressCmd() {
  String msg = (String) "PROGRESS";
  Serial.println(msg);
}

void sendLine(String s) {
  Serial.println(s);
}
void sendEndOfData() {
  Serial.println('.');
}

#ifdef _DEBUG_
void debug(String m) {
  String msg = (String) "DEBUG," + m;
  Serial.println(msg);
}
#else
#define debug(m)
#endif

boolean parseCmd (char*  cmd, String* params, const char* s, byte num) {
  char* p;
  byte i = 0, l = 0;
  String cmdOrig = cmd;
  for (i = 0; i < num - 1; i++) {
    if (i == 0)
      p = strtok (cmd, s);
    else
      p = strtok (NULL, s);
    if (p == NULL)
      return false;
    l += strlen(p) + 1;
    params[i] = p;
  }
  params[num - 1] = cmdOrig.substring(l);;
  return true;
}


void setWifi(String* p) {
  // SETWIFI,ssid,psk
  unsigned long start = millis();
  boolean timeout = false;
  sendStartCmd();

  WiFi.begin(p[0].c_str(), p[1].c_str());

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    sendProgressCmd();
    if (millis() - start > TIMEOUT * 1000) {
      timeout = true;
      break;
    }
  }
  if (timeout) {
    sendStatus(CMDTIMEOUT);
    return;
  }

  debug("IP: " + WiFi.localIP().toString());
  sendStatus(CMDOK);
}


void sslGetPost(String* p) {
  // host,port,url,username,pwd,data
  boolean timeout = false, header = true;
  byte* auth64 = NULL;
  boolean post = false;
  boolean httperr = false;
  int httpstatus;
  httpClient http;

  sendStartCmd();
  /*
    WiFiClientSecure client;
    if (!client.connect(p[0].c_str(), p[1].toInt())) {
      sendStatus(SSLCONNECTERR);
      return;
    }
  */
  http.begin(p[0], p[1].toInt(), p[2], true, "");
  //http.begin(p[0], p[1].toInt(), p[2]);
  if (p[5] != "")
    post = true;
  if (p[4] != "") {
    String auth = p[3] + ":" + p[4];
    byte authSize = auth.length();
    byte authSize64 = authSize * 4 / 3;
    auth64 = (byte*) malloc(sizeof(byte) * authSize64 + 1);
    b64_encode((byte*)auth.c_str(), authSize, auth64, authSize64);
    auth64[authSize64] = '\0';
    http.addHeader("Authorization", (String)"Basic " + (const char*)auth64);
  }
  http.addHeader("Connection", "close");

  if (post) {
    httpstatus = http.POST(p[5]);
  } else {
    httpstatus = http.GET();
  }
  if (httpstatus != 200) {
    if (httpstatus == HTTPC_ERROR_CONNECTION_REFUSED)
      sendStatus(SSLCONNECTERR);
    else
      sendStatus(SSLHTTPERR);
  } else {
    sendStatus(CMDOK);
  }
  sendLine(http.getString());
  sendEndOfData();
  /*
    client.print(String("POST ") + p[2] + " HTTP/1.1\r\n" +
               "Host: " + p[0] + "\r\n");
    if (p[3] != "") {
    client.print((String)"Authorization: Basic " + (const char*)auth64 + "\r\n");
    }
    if (post)
    client.print((String)"Content-Length: " + p[5].length() + "\r\n");
    client.print((String)"User-Agent: ESP-01\r\n\r\n");
    if (post)
    client.print(p[5]);
    //"Connection: close\r\n\r\n");

    debug("Request sent");

    while (client.connected()) {
    String line = client.readStringUntil('\n');
    debug(line);
    if (header) {
      //Check HTTP status code
      if (line.startsWith("HTTP/1.1")) {
        int httpstatus = line.substring(9, 12).toInt();
        if (httpstatus != 200) {
          //sendStatus(SSLGETERR);
          httperr = true;
          //client.stop();
          //return;
        }

      }
      if (line == "\r") {
        debug("headers received");
        header = false;
        if (httperr)
          sendStatus(SSLGETERR);
        else
          sendStatus(CMDOK);
      }
    } else {
      sendLine(line);
    }


    //sendProgressCmd();
    }
    client.stop();
    sendEndOfData();
  */

}

void setup() {
  Serial.begin(19200);
  sendStatus(READY);
}

void loop() {
  String cmd;
  char cmdbuf[MAXCMDSZIE];
  String params[2];

  if (Serial.available() > 0) {
    // read the incoming command
    cmd = Serial.readStringUntil('\n');

    // convert C style char array
    cmd.toCharArray(cmdbuf, MAXCMDSZIE);

    // split command in token separated by ','
    if (!parseCmd(cmdbuf, params, " ", 2)) {
      sendStatus(INVALIDCMD);
    } else {
      params[1].toCharArray(cmdbuf, MAXCMDSZIE);
      //Serial.println(cmdbuf);
      if (params[0] == "SETWIFI") {
        String paramsArray[2]; // username and password
        if (!parseCmd(cmdbuf, paramsArray, ",", 2))
          sendStatus(INVALIDCMD);
        else
          setWifi(paramsArray);
      }
      else if (params[0] == "SSLGET") {
        String paramsArray[6]; // host,port,url,username,pwd,""
        if (!parseCmd(cmdbuf, paramsArray, ",", 5))
          sendStatus(INVALIDCMD);
        else {
          paramsArray[5] = "";
          sslGetPost(paramsArray);
        }
      }
      else if (params[0] == "SSLPOST") {
        String paramsArray[6]; // host,port,url,username,pwd
        if (!parseCmd(cmdbuf, paramsArray, ",", 6))
          sendStatus(INVALIDCMD);
        else
          sslGetPost(paramsArray);
      } else {
        sendStatus(INVALIDCMD);
      }
    }
  }

}

