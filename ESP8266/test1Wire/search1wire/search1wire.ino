#include <OneWire.h>

// Пин, к которому подключен 1-Wire датчик
const int oneWireBus = 5;

OneWire oneWire(oneWireBus);

void setup() {
  Serial.begin(9600);
  discoverOneWireDevices();
}

void loop() {
  // Ничего не делаем в основном цикле
}

void discoverOneWireDevices() {
  byte address[8];
  int deviceCount = 0;

  Serial.println("Searching for 1-Wire devices...");

  oneWire.reset_search();

  while (oneWire.search(address)) {
    Serial.print("Found device ");
    Serial.print(deviceCount);
    Serial.print(" with address: ");
    printAddress(address);
    Serial.println();

    deviceCount++;
  }

  if (deviceCount == 0) {
    Serial.println("No 1-Wire devices found");
  } else {
    Serial.print("Found ");
    Serial.print(deviceCount);
    Serial.println(" 1-Wire devices");
  }
}

void printAddress(byte address[]) {
  for (int i = 0; i < 8; i++) {
    if (address[i] < 16) {
      Serial.print("0");
    }
    Serial.print(address[i], HEX);
    if (i < 7) {
      Serial.print(" ");
    }
  }
}
