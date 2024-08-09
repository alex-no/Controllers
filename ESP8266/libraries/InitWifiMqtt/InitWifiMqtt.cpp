/*
 * InitWifiMqtt Arduino/ESP8266WiFi
 * Version 1.01
 */

#include <InitWifiMqtt.h>
#include <EEPROM.h>
//#include <ESP8266WiFi.h>
//#include <SPI.h>
//#include <Ethernet.h>
//#include <PubSubClient.h>

InitWifiMqtt::InitWifiMqtt(unsigned int pinButton)
  : m_pinButton(pinButton)
//  , m_previousTime(millis())
//  , m_previousStatus(false)
{
  //WiFiClient espClient;
  //PubSubClient client(espClient);
};

/**
 * Button resetPassword:
 *  false - button is not pressed - do nothing
 *  true - button is pressed - reset Wifi password
 */
boolean InitWifiMqtt::checkResetPassword()
{
  unsigned long checkTime = millis() + checkResetDelay;
  do {
    if (digitalRead(m_pinButton) == normalStatus) {
      return false;
    }
  } while (millis() < checkTime);
  return true;
};

String InitWifiMqtt::generateNewPassword()
{
  static String psw;

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  Serial.println("Enter new WiFi password");
  psw = readSerialString();
  digitalWrite(LED_BUILTIN, HIGH);

  saveStringEEPROM(psw);
  Serial.println("new WiFi password saved");
  for (int i = 0; i < 5; i++) {
    Serial.println();
  }

  return psw;
};

boolean InitWifiMqtt::initSerial()
{
  Serial.begin(9600);

  unsigned long checkTime = millis() + checkSerialDelay;
  do {
    if (Serial) {
      Serial.println("");
      return true;
    }
  } while (millis() < checkTime);
  return false;
};

/**
 */
int InitWifiMqtt::init()
{
  delay(startDelay);

  EEPROM.begin(eepromSize);
  boolean isSerial = initSerial();

  if (checkResetPassword()) {
    wifiPassword.clear();
  } else {
    wifiPassword = readStringEEPROM();
  }

  if (wifiPassword.length() == 0 && isSerial) {
    Serial.println("Arduino is connected!");
    wifiPassword = generateNewPassword();
  }

  Serial.println("WiFi password: ");
  Serial.println(wifiPassword.c_str());


  //Serial.println("Connected to the WiFi network");

};

/**
 * Read String from Serial
 */
String InitWifiMqtt::readSerialString()
{
  static String buffer;
  unsigned long checkTime = millis() + readSerialDelay;

  while (millis() < checkTime) {
    if (Serial.available() > 0) {
      char inChar = (char)Serial.read();
      if (inChar == '\n') {
        Serial.println("Stop by press Enter");
        return buffer;
      }
      buffer += inChar;
      if (buffer.length() >= 19) {
        Serial.println("Stop by length password");
        return buffer;
      }
    }
  }

  Serial.println("Stop by timeout");
  return buffer;
};

/**
 * Read String from EEPROM
 */
String InitWifiMqtt::readStringEEPROM()
{
  static String buffer;

  for (int i = 0; i < 20; i++) {
    byte c = EEPROM.read(eepromAddrPassword + i);
    if (c == '\0' || c == 255) {
        break;
    }
    buffer += char(c);
  }

  return buffer;
};

/**
 * Save String to EEPROM
 */
void InitWifiMqtt::saveStringEEPROM(String const& psw)
{
  if (psw.length() >= 20) {
    Serial.println("Error: Password too long");
  } else {
    for (int i = 0; i < psw.length(); i++) {
      EEPROM.write(eepromAddrPassword + i, psw[i]);
    }
    EEPROM.write(eepromAddrPassword + psw.length(), '\0');
    EEPROM.commit();
  }
};
