/*
 * Toilet
 */

#ifndef TOILET_H_
#define TOILET_H_

#include <Arduino.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

extern LiquidCrystal_I2C lcd;

class Toilet
{
  public:
    Toilet();
    void init(); // Init input/uotput
    void measureTemperature(); // Make measure Temperature
    void measurePollution();   // Make measure Pollution
    
    const float m_delta = 0.2;                   // Понижение температуры за указанный интервал, приводящий к включению вентилятора.
    static const int s_total_temp_point = 5;     // Количество точек для измерения температуры (значений)
    static const int s_major_interval_count = 4; // Количество интервалов с понижением, приводящих к срабатывани
    static const int s_pollutionLevel0 = 340;     // Уровень загрязнения воздуха, приводящий к включению вентилятора
    static const int s_pollutionLevel1 = 500;     // Уровень загрязнения воздуха, приводящий к включению сигнала "Alarm"

    int pinFan;      // Пин для подключения реле, управляющего вентилятором туалета
    int pinLedRed;   // Пин для подключения красного светодиода
    int pinLedGreen; // Пин для подключения зелёного светодиода
    int pinLedBlue;  // Пин для подключения синего светодиода

    int pinPollution;   // Пин (аналоговый) для подключения датчика загрязнённости

  protected:
    void _processFan(); // On/off fan and show indicator
    
    float m_temp[s_total_temp_point]; // значения температуры за предыдущие 6 временных интервала
    
    bool m_temperatureDown = false; // Флаг выставляется если температура начала падать (включаем вентилятор и желтый светодиод)
    bool m_pollutionMiddle = false; // Флаг при среднем уровне загрязнения (включаем вентилятор и желтый светодиод)
    bool m_pollutionHeight = false; // Флаг при высоком уровне загрязнения (включаем вентилятор и красный светодиод)
    // Если все три флага сброшены, вентилятор выключен и включен зелёный светодиод
};
#endif //TOILET_H_
