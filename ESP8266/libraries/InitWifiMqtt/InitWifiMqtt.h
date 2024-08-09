/*
 * InitWifiMqtt - Init WiFi, set WiFi password, Init Mqtt
 */

#ifndef INIT_WIFI_MQTT_H_
#define INIT_WIFI_MQTT_H_

#include <ESP8266WiFi.h>

class InitWifiMqtt
{
  public:
    InitWifiMqtt(unsigned int pinButton); // Constructor (pin_button)

    int init(); // Check button status and reyturn its value
    
    int eepromSize = 64;

    int startDelay = 500;        // Delay for all elements to become operational. 
    int checkResetDelay = 350;
    int checkSerialDelay = 5000;
    int readSerialDelay = 10000;
    int serialSpeed = 9600;

    int eepromAddrPassword = 0;

    //bool normalStatus = LOW;         // Status of pin if button is not pressed
    bool normalStatus   = false;       // Status of pin if button is not pressed
    //const String wifiServerIp = "192.168.8.1";  // Address of WiFi router
    IPAddress wifiIp;                  // Address of WiFi connection
    const String wifiName = "my home"; // Time when button pressed double
    String wifiPassword;

    const String mqttServer = "192.168.8.1";
    int mqttPort = 1883;
    const String mqttUser = "";
    const String mqttPassword = "";

  protected:
    boolean checkResetPassword(); // State of button - is reset WiFi Password
    String generateNewPassword();
    boolean initSerial();         // Init Serial conection
    String readSerialString();
    String readStringEEPROM();
    void saveStringEEPROM(String const& psw);
    IPAddress connecWiFi();

    int m_pinButton;                // Counter - how many times button was pressed with shot time interwal
//    unsigned long m_previousTime;   // Time when button was pressed previous time
//    bool m_status;                   // Button status (pressed = true. released = false)
};
#endif //INIT_WIFI_MQTT_H_