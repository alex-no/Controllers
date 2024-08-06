#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Замените следующие значения вашими данными Wi-Fi и MQTT
const char* ssid = "my home";
const char* password = "MyHomeWiFi7";
const char* mqttServer = "192.168.8.1";
const int mqttPort = 1883;
const char* mqttUser = "";
const char* mqttPassword = "";

const char* mqttTopic = "pump";
const char* mqttMessage = "Hello MQTT!";

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to the WiFi network");

  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);

  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
    
    if (client.connect("ESP8266Client", mqttUser, mqttPassword)) {
      Serial.println("Connected to MQTT");
    } else {
      Serial.print("Failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }
}

void loop() {
  client.loop();

  // Отправка сообщения на топик раз в 5 секунд
  static unsigned long lastMillis = 0;
  if (millis() - lastMillis > 5000) {
    client.publish(mqttTopic, mqttMessage);
    lastMillis = millis();
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  // Обработка входящего сообщения, если это необходимо
}
