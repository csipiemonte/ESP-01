#include "espSerial.h"

espSerial* esp;
void setup() {


  Serial.begin(115200);
  esp = new espSerial(10, 11, 4);

  if (esp->setWifi("SID", "PSK")) {
    Serial.println("WifI connected");
  } else {
    Serial.println("ERROR");
  }
  /*
    if (esp->sslGet(body, "istream.smartdatanet.it", 443, "/api/input/sandbox", "test", "test")) {
    Serial.println(body);
    } else {
    Serial.println("ERROR");
    }
  */

}

void loop() {
  Serial.println("Sending json...");
    String body;
  String json = "{ \"sensor\": \"5041b104-35f4-42db-c564-d90db4116305\", \"stream\": \"esptest\", \"values\": [ { \"time\": \"2015-12-02T16:40:03Z\", \"components\": { \"value\": \"50\" } } ] }";

  Serial.println(json);
  if (esp->sslPost(body, "stream.smartdatanet.it", 443, "/api/input/sandbox", json, "sandbox", "sandbox$1")) {
    Serial.println(body);
  } else {
    Serial.println((String)"ERROR\n:"+body);
  }
  delay(2000);

}
