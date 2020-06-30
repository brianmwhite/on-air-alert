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
int lightThresholdValue = 150;

#define PIN_LED_RED 14
#define PIN_LED_GREEN 13
#define PIN_LED_BLUE 12
#define PIN_BUTTON 4

int COLOR_RED[] = {255,0,0};
int COLOR_GREEN[] = {0,255,0};
int COLOR_BLUE[] = {0,0,255};
int *LED_COLOR;

int SERIAL_BAUD_RATE = 115200;
bool alertOn = false;
bool sendLightCommandAgain = false;
int NumberOfSecondsSinceLastCheck = 0;
int NumberOfSecondsToResetAndCheckAgain = 60;

enum AlertModeState
{
    ALERT_MODE_AUTO,
    ALERT_MODE_MANUAL
};

AlertModeState AlertMode = ALERT_MODE_AUTO;

void WifiManagerPortalDisplayedEvent(WiFiManager *myWiFiManager) { Serial.println("DEBUG: Wifi Portal Displayed"); }
void WifiManagerWifiConnectedEvent() { Serial.println("DEBUG: Wifi Connected"); }

void setColor(int red, int green, int blue)
{
  analogWrite(PIN_LED_RED, red);
  analogWrite(PIN_LED_GREEN, green);
  analogWrite(PIN_LED_BLUE, blue);
}

void setColor(int colorValues[]) {
  analogWrite(PIN_LED_RED, *(colorValues + 0));
  analogWrite(PIN_LED_GREEN, *(colorValues + 1));
  analogWrite(PIN_LED_BLUE, *(colorValues + 2));
}

void LED_ON()
{
  setColor(LED_COLOR);
}

void LED_OFF()
{
  setColor(0,0,0);
}

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
  LED_ON();
  Serial.println("turn video alert on");
}

void TurnVideoAlertOff()
{
  alertOn = false;
  CallAPI("http://192.168.7.97:5015/videoalert/off");
  LED_OFF();
  Serial.println("turn video alert off");
}

void setup()
{
  Serial.begin(SERIAL_BAUD_RATE);
  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_LED_BLUE, OUTPUT);
  pinMode(PIN_BUTTON, INPUT_PULLUP);

  LED_COLOR = COLOR_RED;

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
  if (digitalRead(PIN_BUTTON) == LOW)
  {
    Serial.println("button pressed!");
    if (AlertMode == ALERT_MODE_AUTO) {
      AlertMode = ALERT_MODE_MANUAL;
      if (alertOn == true) {
        alertOn = false;
        TurnVideoAlertOff();
        //turn LED white?
      } else {
        alertOn = true;
        TurnVideoAlertOn();
        //turn LED purple?
      }
    } else if (AlertMode == ALERT_MODE_MANUAL) {
      AlertMode = ALERT_MODE_AUTO;
    }
  }
  
  if (AlertMode == ALERT_MODE_AUTO) {
    photocellReading = analogRead(PIN_PHOTOCELL);
    Serial.print("photo sensor = ");
    Serial.print(photocellReading);
    Serial.print(" | alert = ");
    Serial.print(alertOn);
    Serial.print(" | seconds since = ");
    Serial.println(NumberOfSecondsSinceLastCheck);

    NumberOfSecondsSinceLastCheck++;
    if (NumberOfSecondsSinceLastCheck >= NumberOfSecondsToResetAndCheckAgain)
    {
      sendLightCommandAgain = true;
      NumberOfSecondsSinceLastCheck = 0;
    }

    if (photocellReading >= lightThresholdValue && (!alertOn || sendLightCommandAgain))
    {
      TurnVideoAlertOn();
      sendLightCommandAgain = false;
      NumberOfSecondsSinceLastCheck = 0;
    }
    else if (photocellReading < lightThresholdValue && (alertOn || sendLightCommandAgain))
    {
      TurnVideoAlertOff();
      sendLightCommandAgain = false;
      NumberOfSecondsSinceLastCheck = 0;
    }
  }

  
  delay(1000);
}