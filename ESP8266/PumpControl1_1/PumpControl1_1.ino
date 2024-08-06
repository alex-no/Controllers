// Version 1_1
#include <Ticker.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <AlexButton01.h>

const int pinPressureSensor = A0; // A0 Pin for Pressure Sensor
const int pinStreamSensor   = 16; // D0 Pin for check water streram

const int pinButton = 5; // D1 Pin for control button

const int pinLedRed   = 4; // D2 Pin for Red Led
const int pinLedGreen = 0; // D3 Pin for Green Led
const int pinLedBlue  = 2; // D4 Pin for Blue Led
const int pinClock = 14; // D5 Indicator SCLK

const int pinLatch = 12; // D6 Indicator RCLK
const int pinData  = 13; // D7 Indicator DIO

const int pinPump  = 15; // D8 Main Pin for control of Pump

const int pinBuzzer = 3; // SD2 Pin for buzzer

const int minPressureFrom = 150; // Minimal Pressure set From
const int minPressureTo   = 300; // Minimal Pressure set To

const int maxPressureFrom = 300; // Maximum Pressure set From
const int maxPressureTo   = 450; // Maximum Pressure set To

const int minErrorPressure =  10; // Minimal Pressure to Error stopped
const int maxErrorPressure = 440; // Maximum Pressure to Error stopped

const int minPressureSensorVal = 100; // Minimum value from Pressure Sensor (absolute)

const unsigned int eAddrMinPressure      = 0;
const unsigned int eAddrMaxPressure      = 2;
const unsigned int eAddrIsAutoStart      = 4;
const unsigned int eAddrIsControlStream  = 5;
const unsigned int eAddrIsLimitPumpTime  = 6;
const unsigned int eAddrPumpTimeLimit    = 7;

const unsigned int buzzerButtonDuration = 50;   // Duration of buzzer sygnal
const unsigned int buzzerErrorDuration  = 1500; // Duration of buzzer sygnal
const unsigned int buzzerFrequency      = 750;  // Frequency of buzzer sygnal

const int showPressurePeriod = 300;      // Show pressure period
const int stopPumpingDelay = 3000;       // Do not run pump after stop
const int useSettingDelay = 10000;       // Switch to sett0ng mode
const int sensorActiveDelay = 7000;      // Stram is not active
const int emptyPressureDelay = 10000;    // Pressure is empty

String ssid = "";       // Имя вайфай точки доступа
String password = "";   // Пароль от точки доступа
String mqtt_server = "192.168.8.1"; // Имя сервера MQTT
const int mqtt_port = 1883;              // Порт для подключения к серверу MQTT
//String mqtt_user = ""; // Логин от сервера
//String mqtt_pass = ""; // Пароль от сервера

// ===================================== Global variables =====================================
/*
 * Mode values:
 *  0 - stop (wait for press button or MQTT command);
 *  1 - wait for min pressure and switch to 2-mode;
 *  2 - pump water;
 *  4 - setting parameters;
 *  5 - error mode;
 */
unsigned int mode = -1;

/*
 * Error codes:
 * -1 - no errors;
 *  0 - pressure is equal 0 atm when pump is work for a some time;
 *  1 - stream is not exists;
 *  2 - pump is working for a long time without stop;
 *  3 - pressure is too height;
 *  4 - no data from Pressure Sensor;
 */
int errorCode = -1;

int ledInd[4];
int nMap[10] = {3, 159, 37, 13, 153, 73, 65, 27, 1, 9};

int minPressure;
int maxPressure;
int pressure;
int prevPressure;

boolean isAutoStart;
boolean isControlStream;
boolean isLimitPumpTime;
uint8_t pumpTimeLimit;

int buttonStatus;
int sensorStatus;

unsigned long startPumpingTime;
unsigned long stopPumpingTime;
unsigned long sensorActiveTime;

// ---------- Object instances ----------
Ticker ledRefrasher;
AlexButton01 button(pinButton);
AlexButton01 streamSensor(pinStreamSensor);

//#define BUFFER_SIZE 100
WiFiClient espClient;      
PubSubClient mqttClient(espClient); // MQTT client

// ===================================== Root operations =====================================
void setup()
{
//Serial.begin(9600); while (!Serial) { ; }

  pinMode(pinStreamSensor, INPUT_PULLUP);
  pinMode(pinButton, INPUT_PULLUP);

  pinMode(pinLedRed,   OUTPUT);
  pinMode(pinLedGreen, OUTPUT);
  pinMode(pinLedBlue,  OUTPUT);

  pinMode(pinClock, OUTPUT);
  pinMode(pinLatch, OUTPUT);
  pinMode(pinData,  OUTPUT);

  pinMode(pinPump,  OUTPUT);

  ledInd[0] = 255;
  ledInd[1] = 67;
  ledInd[2] = 197;
  ledInd[3] = 255;
  ledRefrasher.attach(0.003, refreshLed);

  button.multipress_limit = 1; 
  streamSensor.normal_status = true; 
  streamSensor.multipress_limit = 1; 
  delay(500);
  
  EEPROM.begin(8);
  
  checkWiFiCredentials();
  setDefaultValues();

  if (getPressure() > 0) {
    changeMode(isAutoStart ? 1 : 0);
  }
  
  if (ssid.length()  > 0) {
    for (int i = 0; i < 5 && !connectWifi(); i++) {
      delay(1000);
    }
    mqttClient.setServer(mqtt_server, mqtt_port);
    mqttClient.setCallback(receiveMqtt);
  }
}

void loop()
{
  pressure = getPressure();

  if (pressure > maxErrorPressure) {
    setErrorMode(3);
  }
//Serial.print("mode=");Serial.println(mode);
  int buttonStatus = button.check();

  switch (mode) {
    case 0:
      processStop(buttonStatus);
      break;
    case 1:
      processWait(buttonStatus);
      break;
    case 2:
      processPumping(buttonStatus);
      break;
    case 4:
      processSettting(buttonStatus);
      break;
    case 5:
      processError(buttonStatus);
      break;
  }

  prevPressure = pressure;

  publishPressure();
//delay(300);
}

// ===================================== Init Main Variables =====================================
void checkWiFiCredentials()
{
  if (button.state()) {
    setWiFiCredentials();
  } else {
    getWiFiCredentials();
  }
  
  if (ssid.length() == 0) {
    // Set new Credentials
    while (!Serial) {}
    
    if (Serial.available() > 0) {
      ssid = Serial.readStringUntil('\n'); // Считать данные до символа новой строки
      Serial.print("WiFi name: ");
      Serial.println(ssid);
    }
    if (Serial.available() > 0) {
      password = Serial.readStringUntil('\n'); // Считать данные до символа новой строки
      Serial.print("WiFi password: ");
      Serial.println(password);
    }
    setWiFiCredentials();
  }
}
void getWiFiCredentials()
{
  ssid = readString(0);
  password = readString(16);
}
void setWiFiCredentials()
{
  writeString(ssid, 0);
  writeString(password, 16);
}

void setDefaultValues()
{
  isAutoStart = EEPROM.read(eAddrIsAutoStart);
  if (isAutoStart < 0 || isAutoStart > 1) {
    isAutoStart = 0;
    EEPROM.write(eAddrIsAutoStart, isAutoStart);
  }
  
  minPressure = readPressure(eAddrMinPressure);
  if (minPressure < minPressureFrom || minPressure > minPressureTo) {
    minPressure = (int)((minPressureFrom + minPressureTo) / 2);
    savePressure(eAddrMinPressure, minPressure);
  }
  maxPressure = readPressure(eAddrMaxPressure);
  if (maxPressure < maxPressureFrom || maxPressure > maxPressureTo) {
    maxPressure = (int)((max(minPressure, maxPressureFrom) + maxPressureTo) / 2);
    savePressure(eAddrMaxPressure, maxPressure);
  }

  isControlStream = EEPROM.read(eAddrIsControlStream);
  if (isControlStream < 0 || isControlStream > 1) {
    isControlStream = 1;
    EEPROM.write(eAddrIsControlStream, isControlStream);
  }
  isLimitPumpTime = EEPROM.read(eAddrIsLimitPumpTime);
  if (isLimitPumpTime < 0 || isLimitPumpTime > 1) {
    isLimitPumpTime = 1;
    EEPROM.write(eAddrIsLimitPumpTime, isLimitPumpTime);
  }
  pumpTimeLimit = EEPROM.read(eAddrPumpTimeLimit);
  if (pumpTimeLimit < 1 || pumpTimeLimit > 30) {
    pumpTimeLimit = 30;
    EEPROM.write(eAddrPumpTimeLimit, pumpTimeLimit);
  }

  EEPROM.commit();
}

// ===================================== Processing functions =====================================
void processStop(int buttonStatus)
{
//Serial.println("processStop");
  if (buttonStatus == 4) {
    changeMode(4);
    return;
  }

  showPressure();
  
  if (buttonStatus == -1 && runPumping()) {
    setAutoStart(true);
  }
}

void processWait(int buttonStatus)
{
//Serial.println("processWait");
  if (buttonStatus == 4) {
    changeMode(4);
    return;
  }

  showPressure();
  
  if (pressure < minPressure && prevPressure < minPressure || buttonStatus == -1) {
    runPumping();
  }
}

void processPumping(int buttonStatus)
{
//Serial.println("processPumping");
  showPressure();
  
  unsigned long currentTime = millis();
  
  // Control Stop Pumping
  if (buttonStatus == 1) {
    stopPumpingTime = currentTime;
    setAutoStart(false);
    changeMode(0);
  } else if (pressure > maxPressure && prevPressure > maxPressure) {
    changeMode(1);
  }

  // Check Stream
  if (isControlStream) {
    int sensorStatus = streamSensor.check();
    if (sensorStatus == 1 || sensorStatus == -1) {
      sensorActiveTime = currentTime;
      digitalWrite(pinLedBlue,  sensorStatus == 1);
    } else if (currentTime - sensorActiveTime > sensorActiveDelay) {
      setErrorMode(1);
    }
  }

  // Check Pressure
  if (currentTime - startPumpingTime > emptyPressureDelay && pressure < 10) {
    setErrorMode(0);
  }

  // Check Time of Pumping
  if (isLimitPumpTime && currentTime - startPumpingTime > pumpTimeLimit * 60000) {
    setErrorMode(2);
  }
}

void processSettting(int buttonStatus)
{
  // 0	- контролируем максимальное время работы насоса L0 - не ограничиваем время L1 - ограничиваем
  // 1	- контроль протока S0 - не контролируем проток, S1 - контролируем проток
  // 2	- минимальное давление (атм.) P_ от 1.5  до 3.5 с шагом 0.1
  // 3	- максимальное давление (атм.) Pˉ̅ от min + 0.1  до 5.0 с шагом 0.1
  // 4	- максимальное время работы насоса (мин.) t от 1  до 30 с шагом 1

  static int setParam = -1;
  static boolean prepare = false;
  static unsigned int lastActive;

  if (buttonStatus != 0 || setParam == -1) {
    lastActive = millis();
  }

  if (buttonStatus == 4 || setParam == -1) {
    setParam++;
    if (setParam > 4) {
      setParam = 0;
    }

    digitalWrite(pinLedRed,   LOW);
    digitalWrite(pinLedGreen, HIGH);
    digitalWrite(pinLedBlue,  HIGH);

    prepare = true;
    showSetting(setParam);
    return;
  }

  if (prepare && buttonStatus == -1) {
    prepare = false;
    digitalWrite(pinLedGreen, LOW);
    digitalWrite(pinLedBlue,  LOW);
    return;
  }

  if (buttonStatus == -1) {
    changeSetting(setParam);
    showSetting(setParam);
    return;
  }

  if (millis() - lastActive > useSettingDelay) {
    setParam = -1;
    setAutoStart(false);
    changeMode(0);
  }
}

void showSetting(int setParam)
{
  clearLed();
  switch (setParam) {
    case 0:
      ledInd[0] = 227;
      ledInd[1] = nMap[isLimitPumpTime ? 1 : 0];
      break;
    case 1:
      ledInd[0] = 73;
      ledInd[1] = nMap[isControlStream ? 1 : 0];
      break;
    case 2:
      showInt(minPressure / 10);
      if (minPressure < 100) {
        ledInd[2] = nMap[0];
      }
      ledInd[2] -= 1;
      ledInd[0] = 49;
      ledInd[1] = 239;
      break;
    case 3:
      showInt(maxPressure / 10);
      if (minPressure < 100) {
        ledInd[2] = nMap[0];
      }
      ledInd[2] -= 1;
      ledInd[0] = 49;
      ledInd[1] = 127;
      break;
    case 4:
      showInt(pumpTimeLimit);
      ledInd[0] = 225;
      break;
  }
}
void changeSetting(int setParam)
{
  switch (setParam) {
    case 0:
      isLimitPumpTime = !isLimitPumpTime;
      EEPROM.write(eAddrIsLimitPumpTime, isLimitPumpTime);
      break;
    case 1:
      isControlStream = !isControlStream;
      EEPROM.write(eAddrIsControlStream, isControlStream);
      break;
    case 2:
      minPressure += 10;
      if (minPressure > minPressureTo) {
        minPressure = minPressureFrom;
      }
      savePressure(eAddrMinPressure, minPressure);
      break;
    case 3:
      maxPressure += 10;
      if (maxPressure > maxPressureTo) {
        maxPressure = maxPressureFrom;
      }
      savePressure(eAddrMaxPressure, maxPressure);
      break;
    case 4:
      pumpTimeLimit++;
      if (pumpTimeLimit > 30) {
        pumpTimeLimit = 1;
      }
      EEPROM.write(eAddrPumpTimeLimit, pumpTimeLimit);
      break;
  }
}

void processError(int buttonStatus)
{
  if (errorCode == 4 || pressure > maxErrorPressure) {
    return;
  }
  if (buttonStatus == -1) {
    changeMode(0);
  }
}

// ===================================== WiFi/MQTT methods =====================================
// ------ Check Connect to MQTT-server and refresh it ------
boolean checkConnect()
{
  if (connectWifi() && connectMQTT()) {
    mqttClient.loop();
    return true;
  }
  return false;
}
// ------ Check WiFi and reconnect if need ------
boolean connectWifi()
{
  if (ssid.length() == 0) {
    return false;
  }
  if (WiFi.status() != WL_CONNECTED) {
    delay(10);
//Serial.println(); Serial.print("Connecting to "); Serial.println(ssid);

    WiFi.begin(ssid, password);
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
      return false;
    }

//Serial.println(""); Serial.print("WiFi connected IP: "); Serial.println(WiFi.localIP());
  }
  return true;
}
// ------ Check MQTT and reconnect if need ------
boolean connectMQTT()
{
  if (!mqttClient.connected()) {
//Serial.print("Attempting MQTT connection...");

    String clientId = "WaterPump";
    if (mqttClient.connect(clientId.c_str())) {
//Serial.println("connected");
      mqttClient.subscribe("yard/waterSupply/pumpControl");
    } else {
//Serial.print("failed, rc="); Serial.print(mqttClient.state());
      return false;
    }
  }
  return true;
}

// ------ Callback-function for receive MQTT messages ------
void receiveMqtt(char* topic, byte* payload, unsigned int length)
{
/*
Serial.print("Message arrived ["); Serial.print(topic); Serial.print("] ");
for (int i = 0; i < length; i++) {
  Serial.print((char)payload[i]);
}
Serial.println();
*/

  if ((char)payload[1] == 'n' && (mode == 0 || mode == 1 || mode == 5 && errorCode < 3)) { 
    if (runPumping()) {
      setAutoStart(true);
    } else {
      // ToDo: Set special status there for time delay
    }
  } else if ((char)payload[1] == 'f' && mode == 1) {
    stopPumpingTime = millis();
    setAutoStart(false);
    changeMode(0);
  }
}
// ------ Publish Pressure to MQTT by difference and time ------
void publishPressure()
{
  static long _prevTime = 0;
  static int _prevPressure = 0;
  char buffer[4];
  long time = millis();

  if (time - _prevTime > 1000 && checkConnect() && abs(pressure - _prevPressure) > 10) {
    itoa(pressure, buffer, 10);
    mqttClient.publish("yard/waterSupply/pressure", buffer);
//Serial.print("pressure = ");Serial.println(pressure);
    _prevPressure = pressure;
    _prevTime = time;
  }
}

// ===================================== Additinal methods =====================================
// ------ Read and write string to EEPROM ------
void writeString(String str, int address)
{
  for (int i = 0; i < str.length() && i < 16; ++i) {
    EEPROM.write(address++, str[i]);
  }
  EEPROM.write(address, '\0'); // завершающий нулевой символ (null-terminated string)
  EEPROM.commit();
}
String readString(int address)
{
  String str = "";
  char ch;
  while (1) {
    ch = EEPROM.read(address++);
    if (ch == '\0') {
      break;
    }
    str += ch;
  }
  return str;
}
// ------ Run Pumping with check protective time ------
void setAutoStart(boolean _isAutoStart)
{
  isAutoStart = _isAutoStart;
  EEPROM.write(eAddrIsAutoStart, isAutoStart);
  EEPROM.commit();
}
boolean runPumping()
{
  if (millis() - stopPumpingTime > stopPumpingDelay) {
    startPumpingTime = millis();
    sensorActiveTime = startPumpingTime;
    changeMode(2);
    return true;
  }
  return false;
}
// ------ Get Value of Pressure from Pressure sensor ------
int getPressure()
{
  static unsigned int result = 200;

  unsigned int tmp = analogRead(pinPressureSensor);
  delay(3);

  if (abs(tmp - analogRead(pinPressureSensor)) < 5) {
    if (tmp < minPressureSensorVal) {
      setErrorMode(4);
      return 0;
    }
    result = tmp * 0.89 - 150;
  }
  
  return result < 0 ? 0 : result;
}
// ------ Show Pressure at the indicator ------
void showPressure()
{
  static unsigned int lastShowTime;
  unsigned int currentTime = millis();
  if (currentTime - lastShowTime > showPressurePeriod) {
    showInt(pressure);
    if (pressure < 10) {
      ledInd[2] = nMap[0];
    }
    if (pressure < 100) {
      ledInd[1] = nMap[0];
    }
    ledInd[1] -= 1;
  
    lastShowTime = currentTime;
  }
}

// --- Save two bytes of pressure value ---
void savePressure(unsigned int addr, int val)
{
  EEPROM.write(addr, val & 255);
  EEPROM.write(addr + 1, val >> 8);
}
// --- Read pressure value by two bytes ---
int readPressure(unsigned int addr)
{
  int result = EEPROM.read(addr + 1) << 8;
  result += EEPROM.read(addr);
  return result;
}
// ------ Set Error Mode ------
void setErrorMode(int m_errorCode)
{
  char buffer[2];
  if (mode != 5 || m_errorCode > errorCode) {
    changeMode(5);
    errorCode = m_errorCode;

    clearLed();
    ledInd[0] = 97;
    ledInd[1] = nMap[errorCode];

    tone(pinBuzzer, buzzerFrequency, buzzerErrorDuration * (errorCode + 1));

    if (checkConnect()) {
      itoa(errorCode, buffer, 10);
      mqttClient.publish("yard/waterSupply/error", buffer);
    }
  }
}
// ------ Change Mode ------
void changeMode(unsigned int m_mode)
{
  if (mode != m_mode) {
    mode = m_mode;
    
    boolean pumpVal = LOW;
    
    boolean redVal   = LOW;
    boolean greenVal = LOW;
    boolean blueVal  = LOW;

    switch (mode) {
      case 0:
        greenVal = HIGH;
        redVal   = HIGH;
        if (checkConnect()) {
          mqttClient.publish("yard/waterSupply/pumpStatus", "stop");
        }
        break;
      case 1:
        greenVal = HIGH;
        if (checkConnect()) {
          mqttClient.publish("yard/waterSupply/pumpStatus", "wait");
        }
        break;
      case 2:
        blueVal = HIGH;
        pumpVal = HIGH;
        if (checkConnect()) {
          mqttClient.publish("yard/waterSupply/pumpStatus", "pumping");
        }
        break;
      case 4:
        redVal   = HIGH;
        greenVal = HIGH;
        if (checkConnect()) {
          mqttClient.publish("yard/waterSupply/pumpStatus", "stop");
        }
        break;
      case 5:
        redVal = HIGH;
        if (checkConnect()) {
          mqttClient.publish("yard/waterSupply/pumpStatus", "error");
        }
        break;
    }
    if (mode != 5) {
      errorCode = -1;
      tone(pinBuzzer, buzzerFrequency, buzzerButtonDuration);
    }

    digitalWrite(pinPump, pumpVal);

    digitalWrite(pinLedRed,   redVal);
    digitalWrite(pinLedGreen, greenVal);
    digitalWrite(pinLedBlue,  blueVal);
  }
}

// ===================================== Show info at the 7-Led indicator  =====================================
// ------ Show Float number at the 7-Led indicator ------
void showFloat(float v, int d)
{
  int v1 = v * pow(10, d);
  showInt(v1);

  int k = 3 - d;
  if (v < 0.0001) {
    v = 0.0001;
  }
  while (v < 1) {
    ledInd[k++] = nMap[0];
    v *= 10;
  }

  ledInd[3 - d] -= 1;
}
// ------ Show Integer number at the 7-Led indicator ------
void showInt(int v)
{
  char buffer[4];
  int shift;

  clearLed();
  itoa(v, buffer, 10);
  shift = 4 - strlen(buffer);
  for (int i = strlen(buffer) - 1; i >= 0; i--) {
    ledInd[i + shift] = nMap[buffer[i]-48];
  }
}
// ------ Clear the 7-Led indicator ------
void clearLed()
{
  for (int i = 0; i < 4; i++) {
    ledInd[i] = 255;
  }
}
// ------ Refresh the 7-Led indicator ------
void refreshLed()
{
  static int n = 16;
  static int i = 0;

  digitalWrite(pinLatch, LOW);
  shiftOut(pinData, pinClock, LSBFIRST, ledInd[i]);
  shiftOut(pinData, pinClock, LSBFIRST, n << i);
  digitalWrite(pinLatch, HIGH);
  if (++i > 3) {
    i = 0;
  }
}