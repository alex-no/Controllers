#include <InitWifiMqtt.h>

const int pinSensor  = 13; // D7 Main Pin for control of Pump

InitWifiMqtt mqtt(pinSensor);

void setup() {

  mqtt.init();
}

void loop() {
}

