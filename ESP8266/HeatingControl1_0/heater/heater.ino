#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>

// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);

const int slaveSelectPin = 15;

float temp1 = 1.01;
float temp2 = 2.02;


void setup() {
  // Инициализация последовательного порта для вывода данных
  Serial.begin(9600);
  
  // Назначаем SS пин
  pinMode(slaveSelectPin, INPUT);
  SPI.begin();

  // Initialize the LCD
  lcd.init();
  // Turn on the backlight
  lcd.backlight();
  lcd.setCursor(0, 0);
}

void loop() {
/*
  lcd.setCursor(0, 0);
  lcd.print("t1=");
  lcd.print(temp1);

  lcd.setCursor(0, 1);
  lcd.print("t2=");
  lcd.print(temp2);

  temp1 += 0.5;
  temp2 += 0.9;

  delay(1000);
*/  
//  if (SPI.available()) {
    char receivedChar = SPI.transfer(0); // Принять данные от мастера
    lcd.print(receivedChar);
//  }

}
