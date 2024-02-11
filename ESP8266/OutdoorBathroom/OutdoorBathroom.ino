/*
   Управление вентиляцией туалета по:
    - уменьшению температуры окружающего воздуха
    - датчику загазованности воздуха
*/
#include <TimeRunner01.h>
#include "Toilet.h"
#include "Shower.h"
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

extern LiquidCrystal_I2C lcd;

LiquidCrystal_I2C lcd(0x27,20,4);

Toilet g_toilet;
Shower g_shower;

  //TimeRunner01 g_tm_check(2500, [](){g_shower.checkJalousieMoving();}, false);
  //g_tm_check.m_skip_overtime = false;


void setup()
{
  lcd.init();
  lcd.backlight();
  lcd.setCursor(3,1);
  lcd.print("Outdoor toilet");
  lcd.setCursor(1,2);
  lcd.print("shower controller");
  delay(1500);
  lcd.clear();

  g_toilet.pinLedRed   = 4; // Пин для подключения красного светодиода
  g_toilet.pinLedGreen = 5; // Пин для подключения зелёного светодиода
  g_toilet.pinLedBlue  = 6; // Пин для подключения синего светодиода
  g_toilet.pinFan      = 7; // Пин для подключения реле, управляющего вентилятором туалета
  
  g_toilet.pinPollution = A0; // Пин (аналоговый) для подключения датчика загрязнённости
  g_toilet.init();

  g_shower.pinHumidityFan   = 8;  // Пин для подключения вентилятора влажности
  g_shower.pinJalousieOpen  = 9;  // Пин подключения открывания жадюзи
  g_shower.pinJalousieClose = 10; // Пин подключения закрывания жадюзи
  g_shower.pinLedRed        = 11; // Пин для подключения красного светодиода
  g_shower.pinLedGreen      = 12; // Пин для подключения зелёного светодиода
  g_shower.pinLedBlue       = 13; // Пин для подключения синего светодиода
  
  g_shower.pinBotton   = A2; // Пин (аналоговый) для подключения кнопок
  g_shower.pinFeedback = A3; // Пин (аналоговый) для получения данных о двигателе коллектора
  g_shower.init();

  TimeRunner01 tm0(3000, [](){g_toilet.measureTemperature();});
  TimeRunner01 tm1(3000, [](){g_toilet.measurePollution();});
  TimeRunner01 tm2(2000, [](){g_shower.measureTemperature();});
  TimeRunner01 tm3(3000, [](){g_shower.measureHumidity();});
  TimeRunner01 tm4(5,    [](){g_shower.checkJalousieMoving();});
  //tm4.m_skip_overtime = false;
  TimeRunner01 tm5(20, [](){g_shower.checkCangeButton();});
  
}

void loop()
{
  TimeRunner01::check();
}
