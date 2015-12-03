
#include "espSerial.h"

void setup() {
  // put your setup code here, to run once:
  String body;
  Serial.begin(115200);
  espSerial* esp = new espSerial(10, 11, 4);
  
  if (esp->setWifi("SID", "psk")) {
    Serial.println("Wifi connected");
  } else {
    Serial.println("ERROR");
  }
  if (esp->sslGet(body, "stream.smartdatanet.it", 443, "/test.json", "test", "test")) {
    Serial.println(body);
  } else {
    Serial.println("ERROR");
  }
  if (esp->sslPost(body, "stream.smartdatanet.it", 443, "/test.php", "POSTDATA", "test", "test")) {
    Serial.println(body);
  } else {
    Serial.println("ERROR");
  }



}

void loop() {
  // put your main code here, to run repeatedly:

}
