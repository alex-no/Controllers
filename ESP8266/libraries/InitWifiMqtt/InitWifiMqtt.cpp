/*
 * InitWifiMqtt Arduino
 * Version 1.01
 */

#include <InitWifiMqtt.h>

InitWifiMqtt::InitWifiMqtt(unsigned int pin_button)
  : m_pin_button(pin_button)
//  , m_previous_time(millis())
//  , m_previous_status(false)
{
};

/**
 * Button resetWifi:
 *  false - button is not pressed - do nothing
 *  true - button is pressed - reset Wifi password
 */
boolean InitWifiMqtt::resetWifi()
{
  return digitalRead(m_pin_button) != normal_status;
};

/**
 */
int InitWifiMqtt::init()
{
  int result = 0;
  return result;
};
