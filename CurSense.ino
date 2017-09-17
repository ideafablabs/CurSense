/*
 * CurSense.ino
 *
 *  Created on: Sep 1, 2017
 *      Author: mlw
 */

#include "NTPClient.h"
#include "currentSense.h"
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#include <ESP8266WebServer.h>
#include <FS.h>

#include "EEPROM.h"

#define THRESHOLD 0
#define THRESHOLD_TIME 2

int timeHour = 0;
int off = 0;

WiFiUDP ntpUDP;

currentSense cs;

// By default 'time.nist.gov' is used with 60 seconds update interval and
// no offset
// set the timezone in the wifi.json

NTPClient *timeClient;
extern int timeZone_;

extern ESP8266WebServer server;

extern void FSBsetup(void);

uint16_t readThreshold(void)
{
  return ((EEPROM.read(THRESHOLD) << 8) | EEPROM.read(THRESHOLD + 1));
}

uint16_t readThresholdTime(void)
{
  return ((EEPROM.read(THRESHOLD_TIME) << 8) | EEPROM.read(THRESHOLD_TIME + 1));
}

void writeThreshold(uint16_t value)
{
  EEPROM.write(THRESHOLD, (value >> 8) & 0xff);
  EEPROM.write(THRESHOLD + 1, value & 0xff);
  EEPROM.commit();
}

void writeThresholdTime(uint16_t value)
{
  EEPROM.write(THRESHOLD_TIME, (value >> 8) & 0xff);
  EEPROM.write(THRESHOLD_TIME + 1, value & 0xff);
  EEPROM.commit();
}

void handleThreshold(void) {
  String thresholdArg = server.arg("threshold");
  uint16_t threshold;
  
  if (thresholdArg != "")
  {
    threshold = atoi(thresholdArg.c_str());
    writeThreshold(threshold);
    cs.setThreshold(threshold);
    server.send(200, "text/plain", "Threshold set");
  }
  else
  {
    // get the threshold and return it
    Serial.println("We got a request for threshold");
   
    String output = "[";

    char tmpBuf[30];
    sprintf(tmpBuf, "%d", cs.getThreshold());
  
    output += "\"threshold\":\"";
    output += String(tmpBuf);
    output += "\""; 

    output += "]";
    server.send(200, "text/json", output);
  }
}

void handleThresholdTime(void) {
  String thresholdTimeArg = server.arg("thresholdTime");
  uint16_t thresholdTime;
  
  if (thresholdTimeArg.c_str() != "")
  {
    thresholdTime = atoi(thresholdTimeArg.c_str());
    writeThresholdTime(thresholdTime);
    cs.setThresholdTime(thresholdTime);
    server.send(200, "text/plain", "Threshold Time set");
  }
  else
  {
    // get the thresholdTime and return it
    Serial.println("We got a request for thresholdTime");
   
    String output = "[";

    char tmpBuf[30];
    sprintf(tmpBuf, "%d", cs.getThresholdTime());
  
    output += "\"thresholdTime\":\"";
    output += String(tmpBuf);
    output += "\""; 

    output += "]";
    server.send(200, "text/json", output);
  }
}

void setup() {
  EEPROM.begin(512);
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
    json += "\"current\":"+String(cs.getCurrent());
    json += "}";
    server.send(200, "text/json", json);
    json = String();
  });
  
#if 1
   //get heap status, analog input value and all GPIO statuses in one json call
  server.on("/all", HTTP_GET, [](){
    String json = "{";
    json += "\"heap\":"+String(ESP.getFreeHeap());
    json += ",\"current\":"+String(cs.getCurrent());
    json += ",\"threshold\":"+String(cs.getThreshold());
    json += ",\"thresholdTime\":"+String(cs.getThresholdTime());
    json += ",\"timeBelowThreshold\":"+String(cs.getTimeBelowThreshold());
    json += ",\"output\":"+String(cs.getOutput());
    json += ",\"time\":\"";
    json += timeClient->getFormattedTime();
    json += "\"}";
    server.send(200, "text/json", json);
    json = String();
  });
#endif

  server.on("/threshold", handleThreshold);
  server.on("/thresholdTime", handleThresholdTime);
  
  /* read values from EEPROM */

  cs.setThreshold(readThreshold());
  cs.setThresholdTime(readThresholdTime());

  /* start my sampler */

  cs.begin();
  
  // call the browser setup first

  FSBsetup();
  
  timeClient = new NTPClient(ntpUDP, timeZone_);

}

extern void FSBloop(void);

void loop()
{
  int curTime = -1;
  
  // make sure we deal with the time
  //Serial.printf("read %d\n", analogRead(A0));
  
  if (timeClient->update())
  {
    curTime = (timeClient->getEpochTime() % 86400L) / 60;
  }

  // we are only going to use minutes to control things so get the minutes since 12:00

  FSBloop();
}

