/*
 * InitWifiMqtt - Read and return all differenn satatus of button (real or sensor)
 */

#ifndef INIT_WIFI_MQTT_H_
#define INIT_WIFI_MQTT_H_

#include <Arduino.h>

class InitWifiMqtt
{
  public:
    InitWifiMqtt(unsigned int pin_button); // Constructor (pin_button)

    boolean resetWifi(); // State of button - is reset WiFi
    int init(); // Check button status and reyturn its value
    
    //bool normal_status = LOW;     // Status of pin if button is not pressed
    bool normal_status   = false; // Status of pin if button is not pressed
    str wifi_ip = '192.168.1.1';  // Address of WiFi router
    str wifi_name = 'my_Name';    // Time when button pressed double

  protected:
    int m_pin_button;                // Counter - how many times button was pressed with shot time interwal
//    unsigned long m_previous_time;   // Time when button was pressed previous time
//    bool m_status;                   // Button status (pressed = true. released = false)
};
#endif //INIT_WIFI_MQTT_H_