#include <SPI.h>
#include <Ethernet.h>
#include "CloudPlugs.h"

#define AUTH_PLUGID "dev-xxxxxxxxxxxxxxxxxx" /**< The device plug ID */
#define AUTH_PASS "your-password" /**< The master password */
#define AUTH_MASTER true

EthernetClient eClient;
CloudPlugs client(eClient);
String response;

void setup() {
  // initialize serial communications at 9600 bps:
  Serial.begin(9600);
  Serial.println("connect to network");
  
  byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
  
  while (Ethernet.begin(mac) != 1) {
    Serial.println("Error getting IP address via DHCP, trying again...");
    delay(5000);
  }

  response = "";
  client.setAuth(AUTH_PLUGID, AUTH_PASS, AUTH_MASTER);
}

void debug(const char* tag, bool res, String& response){
  Serial.print(tag);
  if(res) Serial.print(" ERROR: "); else Serial.print(": ");
  Serial.print(client.getLastHttpResult());
  Serial.print(" - ");
  Serial.println(response);
}

void loop() {
  Serial.print("LOOP: ");

  String body = "{\"data\":";
  body.concat(getTemp());
  body.concat("}");
  
  bool res = client.publishData("temperature", body.c_str(), response);
  debug("PUBLISH", res, response);

  delay(60000);
}

long getTemp() {
  return random(100);
}
