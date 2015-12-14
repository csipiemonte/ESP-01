
#include <ESP8266WiFi.h>
#include <ESP8266httpClient.h>

#include "b64.h"

#define MAXCMDSZIE 512
#define MAXARGSIZE 512
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

  http.begin(p[0], p[1].toInt(), p[2], true, "");
  //http.begin(p[0], p[1].toInt(), p[2]);
  if (p[5] != "")
    post = true;
  if (p[4] != "") {
    String auth = p[3] + ":" + p[4];
    byte authSize = auth.length();
    byte authSize64 = authSize * 4 / 3;
    authSize64 += authSize64 % 4;
    auth64 = (byte*) malloc(sizeof(byte) * authSize64 + 1);
    b64_encode((byte*)auth.c_str(), authSize, auth64, authSize64);
    auth64[authSize64] = '\0';
    http.addHeader("Authorization", (String)"Basic " + (const char*)auth64);

  }
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Connection", "close");


  if (post) {
    httpstatus = http.POST(p[5]);
  } else {
    httpstatus = http.GET();
  }
  if (httpstatus >= 300) {
    if (httpstatus == HTTPC_ERROR_CONNECTION_REFUSED)
      sendStatus(SSLCONNECTERR);
    else
      sendStatus(SSLHTTPERR);
  } else {
    sendStatus(CMDOK);
  }
  sendLine(http.getString());
  sendEndOfData();

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

