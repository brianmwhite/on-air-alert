# on-air-alert

## Description

An alerting mechanism to turn on a "on-air" light if my webcam is on. Uses an Adafruit Feather HUZZAH with ESP8266 with a photocell looking at the webcam's led and a Hue Go.

## Background

Why? Due to the covid-19 shelter in place orders, my whole family has been working from home. I wanted an indicator that could be seen outside of the room I'm using as an office to let my wife and son know that my webcam is on. I looked for software triggers for my macbook but couldn't find anything reliable. I also wanted something non-destructive to attach to my webcam so I decided to use the camera's LED indicator.

## Parts I used

- Adafruit Feather HUZZAH with ESP8266
- Photocell
- Red LED
- 10KΩ resistor (for photocell)
- 150Ω resistor (for led)
- Hue Go
- Sugru

## Notes

### Information Sources

I used Adafruit's excellent tutorials on the [Feather ESP8266](https://learn.adafruit.com/adafruit-feather-huzzah-esp8266) and [photocells](https://learn.adafruit.com/photocells/overview). The code was written using [Visual Studio Code](https://code.visualstudio.com/) with the [Platformio extension](https://platformio.org/install/integration).

### Photocell

I used a photocell I had on hand and attached it using [sugru](https://sugru.com/). I tried electrical tape but that didn't seem to stick well enough. I found a threshold value that worked for my set up but other photocells and environments might have different light values.

### Hue Go

I used a Hue Go as my indicator and to keep things simple on the ESP8266 I used an existing Flask API that I have running on a RaspberryPi.

The code to change the light is in [another repo of mine](https://github.com/brianmwhite/piapi/blob/1d83c63a8e57da92927ccd4d4529d59e9ca5349e/app.py#L38).

I leaned heavily on info from <https://github.com/tigoe/hue-control> and here are the basics:

    #create a 'user' where $HUE_ADDR the IP of the Hue hub
    curl -X POST -d '{"devicetype":"my app"}' http://$HUE_ADDR/api
    #get status of all lights and determine which ID is the light you want to control: 
    curl http://$ADDR/api/$HUE_USER/lights/
    #turn light on
    curl -X PUT -d '{"on":false}' http://$HUE_ADDR/api/$HUE_USER/lights/$LIGHT_ID/state
    #change light to red
    curl -X PUT -d '{"on": true, "bri": 254, "hue": 65202, "sat": 254, "effect": "none", "xy": [0.6817, 0.3036 ], "ct": 153 }' http://$HUE_ADDR/api/$HUE_USER/lights/$LIGHT_ID/state

### Pictures

![Front View](/examples/front-view.jpg)
![Top View](/examples/top-view.jpg)
![Diagram](/examples/on-air-fritzing.png)
![Video](https://media.giphy.com/media/YkyksE6FCZcdOem3Fx/giphy.gif)
