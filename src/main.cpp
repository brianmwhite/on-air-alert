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

#define DEBOUNCE 500

int COLOR_RED[] = {255, 0, 0};
int COLOR_GREEN[] = {0, 255, 0};
int COLOR_BLUE[] = {0, 0, 255};
int COLOR_WHITE[] = {255, 255, 255};
int COLOR_PURPLE[] = {128, 0, 128};
int COLOR_YELLOW[] = {255, 255, 0};

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

void setColor(int colorValues[])
{
  int red = *(colorValues + 0);
  int green = *(colorValues + 1);
  int blue = *(colorValues + 2);

  Serial.print("R=");
  Serial.print(red);
  Serial.print(" | G=");
  Serial.print(green);
  Serial.print(" | B=");
  Serial.println(blue);

  analogWrite(PIN_LED_RED, red);
  analogWrite(PIN_LED_GREEN, green);
  analogWrite(PIN_LED_BLUE, blue);
}

void LED_ON()
{
  setColor(LED_COLOR);
}

void LED_ON(int *color)
{
  setColor(color);
}

void LED_OFF()
{
  setColor(0, 0, 0);
}

bool CallAPI(String url)
{
  bool successful_http_call = false;

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
        successful_http_call = true;
      }
    }
    else
    {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
  }
  return successful_http_call;
}

void TurnVideoAlertOn()
{
  alertOn = true;
  if (CallAPI("http://192.168.7.97:5015/alert/setcolor/red")) 
  {
    LED_ON();
  } 
  else 
  {
    LED_ON(COLOR_WHITE);
  }
  Serial.println("turn video alert on");
}

void TurnVideoAlertOff()
{
  alertOn = false;
  if (CallAPI("http://192.168.7.97:5015/alert/setcolor/off")) 
  {
    LED_OFF();
  }  
  else 
  {
    LED_ON(COLOR_WHITE);
  }
  Serial.println("turn video alert off");
}

void TurnManualAlertOn()
{
  alertOn = true;
  if (CallAPI("http://192.168.7.97:5015/alert/setcolor/red")) 
  {
    LED_ON(COLOR_PURPLE);
  }
  else 
  {
    LED_ON(COLOR_WHITE);
  }  
  Serial.println("[override] turn video alert on");
}

void TurnManualAlertOff()
{
  alertOn = false;
  if (CallAPI("http://192.168.7.97:5015/alert/setcolor/off"))
  {
    LED_ON(COLOR_GREEN);
  }
  else 
  {
    LED_ON(COLOR_WHITE);
  }
  Serial.println("[override] turn video alert off");
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

  if (wifiManager.autoConnect("Onairalert_4da994b3"))
  {
    Serial.println("DEBUG: WifiManager reports true");
  }
  else
  {
    Serial.println("DEBUG: WifiManager reports false");
  }

  TurnVideoAlertOff();
}

void loop()
{
  if (digitalRead(PIN_BUTTON) == LOW)
  {
    Serial.println("Button pressed");
    delay(DEBOUNCE);
    NumberOfSecondsSinceLastCheck = 0;

    if (AlertMode == ALERT_MODE_AUTO)
    {
      Serial.println("-- Switching to manual mode");
      AlertMode = ALERT_MODE_MANUAL;
      if (alertOn == true)
      {
        TurnManualAlertOff();
        Serial.println("-- Override: alert off");
      }
      else
      {
        TurnManualAlertOn();
        Serial.println("-- Override: alert on");
      }
    }
    else if (AlertMode == ALERT_MODE_MANUAL)
    {
      AlertMode = ALERT_MODE_AUTO;
      sendLightCommandAgain = true;
      Serial.println("-- Switching to auto mode");
    }
  }
  
  photocellReading = analogRead(PIN_PHOTOCELL);
  Serial.print("mode = ");

  if (AlertMode == ALERT_MODE_AUTO)
  {
    Serial.print("auto");
  }
  else
  {
    Serial.print("manual");
  }
  Serial.print(" | photo sensor = ");
  Serial.print(photocellReading);
  Serial.print(" | threshold = ");
  Serial.print(lightThresholdValue);
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
  if (AlertMode == ALERT_MODE_AUTO) 
  {
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
  else if (AlertMode == ALERT_MODE_MANUAL && sendLightCommandAgain)
  {
    if (alertOn)
    {
      TurnManualAlertOn();
      sendLightCommandAgain = false;
      NumberOfSecondsSinceLastCheck = 0;
    }
    else
    {
      TurnManualAlertOn();
      sendLightCommandAgain = false;
      NumberOfSecondsSinceLastCheck = 0;
    }
  }
  
  delay(1000);
}