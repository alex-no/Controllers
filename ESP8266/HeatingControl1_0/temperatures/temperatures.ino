#include <OneWire.h>
#include <DallasTemperature.h>
#include <SPI.h>

// Пин, к которому подключены датчики температуры
#define ONE_WIRE_BUS 5 //D1
#define SPI_CS 15 //D1

// Создаем объект для работы с OneWire
OneWire oneWire(ONE_WIRE_BUS);

// Создаем объект для работы с датчиками температуры
DallasTemperature sensors(&oneWire);

// Уникальные адреса датчиков
DeviceAddress sensor1 = {0x28, 0xFF, 0x26, 0x61, 0x51, 0x17, 0x04, 0x00};
DeviceAddress sensor2 = {0x28, 0xFF, 0xCB, 0xA8, 0x50, 0x17, 0x04, 0x37};

void setup() {
  // Инициализация последовательного порта для вывода данных
  Serial.begin(9600);
  
 	pinMode(SPI_CS, OUTPUT);
	digitalWrite(SPI_CS, HIGH);
	SPI.begin();
	sendStr("qwqwe");

  
  // Инициализация датчиков температуры
  sensors.begin();
  
  // Указываем адреса датчиков
  sensors.setResolution(sensor1, 12);
  sensors.setResolution(sensor2, 12);
}

void loop() {
  // Запросить данные с датчиков
  sensors.requestTemperatures();
  
  // Получить температуру от первого датчика
  float temp1 = sensors.getTempC(sensor1);

  // Получить температуру от второго датчика
  float temp2 = sensors.getTempC(sensor2);

  // Вывести результаты в последовательный порт
  Serial.print("Temperature 1: ");
  Serial.print(temp1);
  Serial.println(" °C");

  Serial.print("Temperature 2: ");
  Serial.print(temp2);
  Serial.println(" °C");

  // Задержка перед следующим измерением
  delay(1000);
}

void sendStr(String tySend)
{
	digitalWrite(SPI_CS, LOW);
	int strLenht = tySend.length();
	for (int i = 0; i < strLenht; i++)
	{		
		SPI.transfer(tySend[i]);		
		delay(10);
	}
	digitalWrite(SPI_CS, HIGH);
}

