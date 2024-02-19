#include <ESP8266WiFi.h> 
#include <OneWire.h>
#include <DallasTemperature.h>

const char* ssid = "192.168.8.1";
const char* password = "******";
#define ONE_WIRE_BUS 5 //D1
OneWire oneWire(ONE_WIRE_BUS);//создаем объект для работы с библиотекой OneWire
DallasTemperature DS18B20 (&oneWire);//создаем объект для работы с библиотекой DallasTemperature

char temperatureC[6];
WiFiServer server (80);// Номер порта для сервера

void getTemperature(){
  float tempC;
  do{
    DS18B20.requestTemperatures();
    tempC = DS18B20.getTempCByIndex(0);
    dtostrf(tempC, 2, 2, temperatureC);
    delay(100);
   }
  while (tempC == 85.0 || tempC == (-127.0));// код 85 - ошибка чтения, -127 - отсутствует датчик
}

void setup()
{
  Serial.begin(9600);
  delay(10);
  DS18B20.begin();//начинаем работу с датчиком
  DS18B20.setResolution(12);//устанавливаем расширение датчика (от 9 до 12)
  Serial.println();
  Serial.print("Start!!!!!!!!!");  
  Serial.println("Connecting to");
  Serial.println(ssid);
  WiFi.begin(ssid,password);
  while (WiFi.status() != WL_CONNECTED){
     delay(500);
     Serial.print(".");
    }
  Serial.println("");
  Serial.println("WiFi connected");
  server.begin();//ожидаем соединения клиентов
  Serial.println(" Web server running. IP:");
  delay(1000);
  Serial.println(WiFi.localIP());
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  pinMode(ONE_WIRE_BUS, INPUT);     // Initialize the ONE_WIRE_BUS pin as an input
}

void loop()
{
  WiFiClient client =server.available();//ожидаем подключение клиентов
  if (client) {// если есть подключение
    while (client.connected()) {// пока клиент подключен к серверу
      if(client.available()) {// если есть не прочитанные данные (возвращает количество непрочитанных байт)
        char c = client.read();// читаем байт или символ, если нечего читать возвращает -1
        if(c == '\n') {// если есть переход на новую строчку 
          getTemperature();// читаем температуру
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println();
          client.println("ESP8266-Temperature ");
          client.println("<br>");
          client.println("Temperature:");
          client.println(temperatureC);
          client.println("*C");
          Serial.print("Temperature =");
          Serial.println(temperatureC);
          break;
        }
      }
    }
    delay(1);
    client.stop();// закрываем соединение
    Serial.println("Client disconnected.");
  }
}
