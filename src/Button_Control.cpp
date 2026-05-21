#include "Button_Control.h"

Button_Control::Button_Control(int startPin, int stopPin)
    : _startPin(startPin), _stopPin(stopPin) {}

void Button_Control::begin() {
    pinMode(_startPin, INPUT_PULLUP);
    pinMode(_stopPin,  INPUT_PULLUP);
    Serial.println("Button_Control: GP20=start GP21=stop");
}

bool Button_Control::isStartPressed() {
    int current = digitalRead(_startPin);
    bool pressed = (current == LOW && _prevStart == HIGH);
    _prevStart = current;
    return pressed;
}

bool Button_Control::isStopPressed() {
    int current = digitalRead(_stopPin);
    bool pressed = (current == LOW && _prevStop == HIGH);
    _prevStop = current;
    return pressed;
}