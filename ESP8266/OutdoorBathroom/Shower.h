/*
 * Shower
 */

#ifndef SHOWER_H_
#define SHOWER_H_

#include <Arduino.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

extern LiquidCrystal_I2C lcd;

class Shower
{
  public:
    Shower();
    void init(); // Init input/uotput

    void measureHumidity();     // Make measure Humidity
    void measureTemperature();  // Make measure Colector Temperature
    void checkJalousieMoving(); // Check is Jalousie Moving
    void checkCangeButton();    // Check Button of Change mode is pushed
    
    static const int s_mode_failure = -1; // Аварийный режим жалюзи
    static const int s_mode_stop    =  0; // Режим жалюзи - выключено управление
    static const int s_mode_open    =  1; // Режим жалюзи - постоянно открыто
    static const int s_mode_close   =  2; // Режим жалюзи - постоянно закрыто
    static const int s_mode_auto    =  3; // Режим жалюзи - автоматическое управление

    static const int s_status_unknown =  0; // Статус жалюзи - неопределено
    static const int s_status_open    =  1; // Статус жалюзи - открыты
    static const int s_status_close   =  2; // Статус жалюзи - закрыты

    static const int s_eeprom_mode_adr = 0;   // EEPROM-адрес для хранения последнего mode жалюзи
    const unsigned long s_delay_press = 3000; // Задержка после нажатия кнопки для применения режима
    const float s_top_colector_temp = 31.0;   // Максимальная температура коллектора - закрывание коллектора
    const float s_low_colector_temp = 29.0;   // Миниимальная температура коллектора - открывание коллектора
    const float s_top_humidity = 41.0;        // Уровень влажности, приводящий к включению вентилятора
    const float s_low_humidity = 38.0;        // Уровень влажности, при котором вентилятор выключается
    
    int pinHumidityFan;   // Пин для подключения вентилятора влажности
    int pinJalousieOpen;  // Пин подключения открывания жадюзи
    int pinJalousieClose; // Пин подключения закрывания жадюзи

    int pinLedRed;   // Пин для подключения красного светодиода
    int pinLedGreen; // Пин для подключения зелёного светодиода
    int pinLedBlue;  // Пин для подключения синего светодиода

    int pinBotton;   // Пин (аналоговый) для подключения кнопок
    int pinFeedback; // Пин (аналоговый) для получения данных о двигателе коллектора

  protected:
    void _afterChangedMode(); // Operation after changed mode
    void _openJalousie();     // Perform Open Jalousie
    void _closeJalousie();    // Perform Close Jalousie
    void _startMoving();      // Stop Jalousie Moving
    void _stopMoving();       // Stop Jalousie Moving
    void _showMode();         // Show Jalousie Mode

    int m_is_jalousie_status  = 0;     // Текущий статус жалюзи: открыты/закрыты/неопределено
    bool m_is_jalousie_moving = false; // Подано напряжение - жалюзи закрываются или открываются
    //int m_moving_start_time   = 0;     // Начало перемещения жалюзи для контроля
    int m_moving_stage = 0; // 
    
    bool m_changing_mode = false;  // Происходит изменение режима
    int m_current_jalousie_mode  = 0; // Текущий режим работы жалюзи
    
};
#endif //SHOWER_H_
