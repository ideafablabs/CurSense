/*
 * CurSense.ino
 *
 *  Created on: Sep 1, 2017
 *      Author: mlw
 */

#include "NTPClient.h"
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#include <ESP8266WebServer.h>
#include <FS.h>
#include "Ticker.h"

int timeHour = 0;
int off = 0;

WiFiUDP ntpUDP;

Ticker currentCheck;

// By default 'time.nist.gov' is used with 60 seconds update interval and
// no offset
// set the timezone in the wifi.json

NTPClient *timeClient;
extern int timeZone_;

extern ESP8266WebServer server;

extern void FSBsetup(void);

/* from measuring */
#define ANALOG_BASE 539;

uint16_t curMax = 0;

void currentSample()
{
  int16_t value;
  
  /* we need to read the analog value and keep a 20 MAX/MIN samples for an average */

  value = analogRead(A0) - ANALOG_BASE;

  if (value < 0)
  {
    value = -value;
  }

  /* now we have the absolute value */

  if ( value > curMax)
  {
    curMax = value;
  }
  else
  {
    /* I think we need to decrement this so we will get a new max at some point */
    if (curMax > 0)
    {
      curMax--;
    }
  }

  if (curMax > 280)
  {
    digitalWrite(0, HIGH);
  }
  else if( curMax < 100)
  {
    digitalWrite(0, LOW);
  }
}

void setup() {
  pinMode(0, OUTPUT);
  digitalWrite(0, LOW);
  Serial.begin(115200);

  // setup the SPIFFS first so we can read our config file 
  
  SPIFFS.begin(); 

  server.on("/heap", HTTP_GET, [](){
    String json = "{";
    json += "\"heap\":"+String(ESP.getFreeHeap());
    json += "}";
    server.send(200, "text/json", json);
    json = String();
  });
  server.on("/current", HTTP_GET, [](){
    String json = "{";
    json += "\"current\":"+String(curMax);
    json += "}";
    server.send(200, "text/json", json);
    json = String();
  });
  
#if 1
   //get heap status, analog input value and all GPIO statuses in one json call
  server.on("/all", HTTP_GET, [](){
    String json = "{";
    json += "\"heap\":"+String(ESP.getFreeHeap());
    json += ",\"current\":"+String(curMax);
    json += ",\"time\":\"";
    json += timeClient->getFormattedTime();
    json += "\"}";
    server.send(200, "text/json", json);
    json = String();
  });
#endif

  /* start my sampler */

  currentCheck.attach(.01, currentSample);
  
  /* stop the test for now
  
  myLeds->ledTest();
  */
  
  // call the browser setup first

  FSBsetup();
  
  timeClient = new NTPClient(ntpUDP, timeZone_);

}

extern void FSBloop(void);

void loop()
{
  int curTime = -1;
  
  // make sure we deal with the time
  
  if (timeClient->update())
  {
    curTime = (timeClient->getEpochTime() % 86400L) / 60;
  }

  // we are only going to use minutes to control things so get the minutes since 12:00

  FSBloop();
}

