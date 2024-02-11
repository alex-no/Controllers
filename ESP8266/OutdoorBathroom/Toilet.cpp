/*
 * Toilet
 */

#include "Toilet.h"
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 3

OneWire oneWire_t(ONE_WIRE_BUS);
DallasTemperature sensor_temp_t(&oneWire_t);

// Toilet mesure
Toilet::Toilet()
{
  sensor_temp_t.begin();
  sensor_temp_t.requestTemperatures();
  float currentTemp = sensor_temp_t.getTempCByIndex(1);

  for (int i = 0; i < s_total_temp_point; i++) {
    m_temp[i] = currentTemp - (i / 10.0);
  }
};

void Toilet::init()
{
  pinMode(pinFan, OUTPUT);
  pinMode(pinLedRed, OUTPUT);
  pinMode(pinLedGreen, OUTPUT);
  pinMode(pinLedBlue, OUTPUT);

  digitalWrite(pinLedGreen, HIGH);
  
}

void Toilet::measureTemperature()
{
  int decline = 0;
  float average = m_temp[0];
  
  sensor_temp_t.requestTemperatures();
  float currentTemp = sensor_temp_t.getTempCByIndex(1);
  
  for (int i = 1; i < s_total_temp_point; i++) {
    if (m_temp[i - 1] > m_temp[i]) {
      decline++;
    }
    average += m_temp[i];
    m_temp[i - 1] = m_temp[i];
  }
  m_temp[s_total_temp_point - 1] = currentTemp;
  average /= s_total_temp_point;

  //m_temperatureDown = (average - currentTemp > m_delta) || (decline >= s_major_interval_count);
  m_temperatureDown = average - currentTemp > (m_temperatureDown ? 0 : m_delta);
  _processFan();
};

void Toilet::measurePollution()
{
  int pollutionLevel = analogRead(pinPollution);
  
  m_pollutionHeight = pollutionLevel >= s_pollutionLevel1;
  m_pollutionMiddle = pollutionLevel >= s_pollutionLevel0;

  _processFan();

  lcd.setCursor(0,0);
  lcd.print("Pollution ");
  lcd.print(pollutionLevel);
  lcd.print("  ");
};

void Toilet::_processFan()
{
  digitalWrite(pinFan, m_temperatureDown || m_pollutionMiddle || m_pollutionHeight ? HIGH : LOW);
  
  boolean led[] = {LOW, LOW, LOW};
  if (m_temperatureDown && !m_pollutionMiddle && !m_pollutionHeight) {
    led[2] = HIGH; // Blue
  } else if (m_pollutionHeight) {
    led[0] = HIGH; // Red
  } else if (m_pollutionMiddle) {
    led[0] = HIGH; // Yellow
    led[1] = HIGH;
  } else {
    led[1] = HIGH; // Yellow
  }

  digitalWrite(pinLedRed,   led[0]);
  digitalWrite(pinLedGreen, led[1]);
  digitalWrite(pinLedBlue,  led[2]);

}
