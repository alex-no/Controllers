/*
 * Shower
 */

#include "Shower.h"
#include <EEPROM.h>
#include <TroykaDHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 3

DHT dht(2, DHT22);

OneWire oneWire_s(ONE_WIRE_BUS);
DallasTemperature sensor_temp_s(&oneWire_s);

// =============== Shower Constructor =============== //
Shower::Shower()
{
  dht.begin();

  sensor_temp_s.begin();
  sensor_temp_s.requestTemperatures();
  
};

// =============== Init Shower =============== //
void Shower::init()
{
  pinMode(pinHumidityFan, OUTPUT);
  pinMode(pinJalousieOpen, OUTPUT);
  pinMode(pinJalousieClose, OUTPUT);

  pinMode(pinLedRed,   OUTPUT);
  pinMode(pinLedGreen, OUTPUT);
  pinMode(pinLedBlue,  OUTPUT);

  m_current_jalousie_mode = EEPROM.read(s_eeprom_mode_adr);
  if (m_current_jalousie_mode < s_mode_stop || m_current_jalousie_mode > s_mode_auto) {
    m_current_jalousie_mode = s_mode_stop;
    EEPROM.update(s_eeprom_mode_adr, m_current_jalousie_mode);
  }
  _afterChangedMode();
}

// =============== Collector Control =============== //
// -------------- Measure Temperature in Collector -------------- //
void Shower::measureTemperature()
{
  float currentTemp = sensor_temp_s.getTempCByIndex(0);
  if (m_current_jalousie_mode == s_mode_auto && !m_changing_mode) {
    if (currentTemp >= s_top_colector_temp) {
      _closeJalousie();
    } else if (currentTemp < s_low_colector_temp) {
      _openJalousie();
    }
  }
  
  lcd.setCursor(0,2);
  lcd.print("Collector t=");
  lcd.print(currentTemp);
};

// -------------- Chek is Jalousie Moving -------------- //
void Shower::checkJalousieMoving()
{
  static unsigned long m_moving_start_time; // Время начала перемещения жалюзи
  unsigned long m_current_last_time; // Время когда последний раз зафиксирован ток в цепи
  
  switch (m_moving_stage) {
    case 0:
      // Выставляем начальные значения и переходим к следующему этапу
      m_moving_start_time = millis();
      m_moving_stage = 1;
    case 1:
      // Ждём появление тока в цепи жалюзи
      if (analogRead(pinFeedback) > 512) {  // Если ток появилось - переходим к следующему этапу
        m_current_last_time = millis();
        m_moving_stage = 2;
      } else if (millis() - m_moving_start_time > 700) { // Если за 0.7 сек ток не начался, значит Жалюзи находятся в граничном положении
        m_moving_stage = -1;
        _stopMoving();
      }
      break;
    case 2:
      // Ждём, пока ток прекратится
      if (analogRead(pinFeedback) > 512) {  // Если ток есть фиксируем новое время и проверяем общее время процесса
        if (millis() - m_moving_start_time > 10000) {
          // Если общее время превысило 10 сек - переходим в режим fail
          m_moving_stage = -1;
          m_current_jalousie_mode = s_mode_failure;
          m_is_jalousie_status    = s_status_unknown;
          _stopMoving();
          _showMode();
          EEPROM.update(s_eeprom_mode_adr, s_mode_stop);
          break;
        }
        m_current_last_time = millis();
      } else if (millis() - m_current_last_time > 200) { // Если за 0.2 сек ток не начался, значит Жалюзи перешли в граничное положение
        m_moving_stage = -1;
        _stopMoving();
      }
      break;
  }
};

// -------------- Chek press control button -------------- //
void Shower::checkCangeButton()
{
  static unsigned long last_press_time;    // Время последнего нажатия кнопки
  static bool prev_status = false;         // Состояние кноопки - нажата
  static int m_previous_jalousie_mode = 0; // Пердыдущий режим работы жалюзи
  
  if (m_current_jalousie_mode != s_mode_failure) {
    // Смена режима разрешена
    bool cur_status = analogRead(pinBotton) < 528;
    
    if (cur_status != prev_status) {
      // Текущее состояние кнопки отлиается от текущего
      m_changing_mode = true;
      prev_status = cur_status;
      if (cur_status) {
        // Меняем статус
        m_current_jalousie_mode++;
        if (m_current_jalousie_mode > s_mode_auto) {
          m_current_jalousie_mode = s_mode_stop;
        }
        _showMode();
      }
    }
    
    if (m_changing_mode) {
      // Мы находимся в состоянии изменения статуса
      if (m_is_jalousie_moving) {
        m_is_jalousie_status = s_status_unknown;
        _stopMoving();
      }

      if (cur_status) {
        // Если кнопка нажата
        last_press_time = millis();
      } else if (millis() - last_press_time >= s_delay_press) {
        // Если кнопка НЕ нажата и время ожидания истекло - применить изменения
        if (m_previous_jalousie_mode != m_current_jalousie_mode) {
          _afterChangedMode();
        }
        EEPROM.update(s_eeprom_mode_adr, m_current_jalousie_mode);
        m_previous_jalousie_mode = m_current_jalousie_mode;
        m_changing_mode = false;
      }
    }
  }
};

// ------ Operation after changed mode ------ //
void Shower::_afterChangedMode()
{
  switch (m_current_jalousie_mode) {
    case s_mode_stop:
      digitalWrite(pinLedRed,   HIGH);
      digitalWrite(pinLedGreen, HIGH);
      digitalWrite(pinLedBlue,  LOW);
      m_is_jalousie_status = s_status_unknown;
      break;
    case s_mode_open:
      _openJalousie();
      break;
    case s_mode_close:
      _closeJalousie();
      break;
    case s_mode_auto:
      measureTemperature();
      break;
  }

  _showMode();
};

// ------ Open Jalousie ------ //
void Shower::_openJalousie()
{
  if (m_is_jalousie_status != s_status_open && !m_is_jalousie_moving) {
    m_is_jalousie_status = s_status_open;
    _startMoving();
  }
};
// ------ Close Jalousie ------ //
void Shower::_closeJalousie()
{
  if (m_is_jalousie_status != s_status_close && !m_is_jalousie_moving) {
    m_is_jalousie_status = s_status_close;
    _startMoving();
  }
};
// ------ Start Jalousie Moving ------ //
void Shower::_startMoving()
{
    m_is_jalousie_moving = true;
    //m_moving_start_time  = millis();
    m_moving_stage = 0;

    digitalWrite(pinLedRed,   LOW);
    digitalWrite(pinLedGreen, LOW);
    digitalWrite(pinLedBlue,  HIGH);

    digitalWrite(pinJalousieOpen,  m_is_jalousie_status == s_status_open);
    digitalWrite(pinJalousieClose, m_is_jalousie_status == s_status_close);
    //g_tm_check.enable();
};

// ------ Stop Jalousie Moving ------ //
void Shower::_stopMoving()
{
    m_is_jalousie_moving = false;

    digitalWrite(pinLedRed,   m_is_jalousie_status == s_status_close || m_current_jalousie_mode == s_mode_stop);
    digitalWrite(pinLedGreen, m_is_jalousie_status == s_status_open  || m_current_jalousie_mode == s_mode_stop);
    digitalWrite(pinLedBlue,  LOW);

    digitalWrite(pinJalousieOpen,  LOW);
    digitalWrite(pinJalousieClose, LOW);
};

// ------ Show Current Mode ------ //
void Shower::_showMode()
{
  lcd.setCursor(0,1);
  lcd.print("Jalousie mode: ");
  switch (m_current_jalousie_mode) {
    case s_mode_failure:
      lcd.print("Fail ");
      break;
    case s_mode_stop:
      lcd.print("Stop ");
      break;
    case s_mode_open:
      lcd.print("Open ");
      break;
    case s_mode_close:
      lcd.print("Close");
      break;
    case s_mode_auto:
      lcd.print("Auto ");
      break;
  }
};


// =============== Humidity Control =============== //
void Shower::measureHumidity()
{
  static bool m_is_humidity_fan_on  = false; // Вентилятор для снижения влажности включен
  
  dht.read();
  if (dht.getState() == DHT_OK) {
    float h = dht.getHumidity();
    if (h > s_top_humidity && !m_is_humidity_fan_on) {
      m_is_humidity_fan_on = true;
      digitalWrite(pinHumidityFan, HIGH);
    } else if (h < s_low_humidity && m_is_humidity_fan_on) {
      m_is_humidity_fan_on = false;
      digitalWrite(pinHumidityFan, LOW);
    }

    lcd.setCursor(0,3);
    lcd.print("Humidity H=");
    lcd.print(h);
  }
};
