#include <InitWifiMqtt.h>

const int pinSensor  = 15; // D8 Main Pin for control of Pump

InitWifiMqtt mqtt(pinSensor);

void setup() {

  mqtt.init();
}

void loop() {
}

