#ifndef LEDContainer_H
#define LEDContainer_H

class LEDContainer
{
public:
    enum LEDState
    {
        ON,
        OFF,
        BLINK,
    };
    LEDContainer();
    void init(int pin);
    void init(int pin, int blink_delay_in_milliseconds);
    void setStatus(LEDState state);
    void setStatus(bool isOk);
    void updateLED();

private:
    int PIN;
    int NEXT_TIME_TO_BLINK;
    int LED_SIGNAL;
    int BLINK_DELAY_MILLISECONDS;
    LEDState READY_STATE;
    void blinkLED();
};

#endif