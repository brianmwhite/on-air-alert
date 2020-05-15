#include <Arduino.h>
#include <ESP8266WiFi.h>

//needed for wifimanager
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h> //https://github.com/tzapu/WiFiManager

#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

#include "LEDContainer.h"

#define PIN_PHOTOCELL 0
int photocellReading;
int lightThresholdValue = 250;

#define PIN_LED_STATUS 14
LEDContainer LED_Status;

int SERIAL_BAUD_RATE = 115200;
bool alertOn = false;

void WifiManagerPortalDisplayedEvent(WiFiManager *myWiFiManager) { Serial.println("DEBUG: Wifi Portal Displayed"); }
void WifiManagerWifiConnectedEvent() { Serial.println("DEBUG: Wifi Connected"); }

void CallAPI(String url) 
{
    WiFiClient client;
    HTTPClient http;

    Serial.print("[HTTP] begin...\n");
    if (http.begin(client, url))
    {
      Serial.print("[HTTP] GET...\n");
      // start connection and send HTTP header
      int httpCode = http.GET();

      // httpCode will be negative on error
      if (httpCode > 0)
      {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
        {
          String payload = http.getString();
          Serial.println(payload);
        }
      }
      else
      {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      }

      http.end();
    }
}

void TurnVideoAlertOn() 
{
    alertOn = true;
    CallAPI("http://192.168.7.97:5015/videoalert/on");
    LED_Status.setStatus(LED_Status.ON);
    Serial.println("turn video alert on");
}

void TurnVideoAlertOff() 
{
    alertOn = false;
    CallAPI("http://192.168.7.97:5015/videoalert/off");
    LED_Status.setStatus(LED_Status.OFF);
    Serial.println("turn video alert off");
}

void setup()
{
  Serial.begin(SERIAL_BAUD_RATE);
  // delay(5000); //for debugging purposes, enough time to start the serial console
  LED_Status.init(PIN_LED_STATUS);

  WiFiManager wifiManager;
  wifiManager.setAPCallback(WifiManagerPortalDisplayedEvent);
  wifiManager.setSaveConfigCallback(WifiManagerWifiConnectedEvent);

  //TODO: maybe create a random suffix using https://github.com/marvinroger/ESP8266TrueRandom

  if (wifiManager.autoConnect("Onairalert_4da994b3")) { Serial.println("DEBUG: WifiManager reports true"); }
  else { Serial.println("DEBUG: WifiManager reports false"); }

  TurnVideoAlertOff();
}

void loop()
{
  photocellReading = analogRead(PIN_PHOTOCELL);
  Serial.print("photo sensor reading = ");
  Serial.print(photocellReading);
  Serial.print(" alert = ");
  Serial.println(alertOn);

  if (photocellReading >= lightThresholdValue && !alertOn)
  {
    TurnVideoAlertOn();
  }
  else if (photocellReading < lightThresholdValue && alertOn)
  {
    TurnVideoAlertOff();
  }
  delay(1000);
}