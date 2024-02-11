#include <EEPROM.h>
#include <AlexButton01.h>

const int pinSensor      = 16; // Pin for control Sensor
const int pinExtraSensor = 5;  // Pin for extra Sensor (moving)
const int pinAutoOn      = 14; // Pin for switcher, allowed automatic on light after power is restored
const int pinAllowBuzzer = 12; // Pin for switcher, allowed automatic on light after power is restored

const int pinBuzzer    = 4; // Pin for buzzer
const int pinLight     = 0; // Pin for control of Light
const int pinIndicator = 2; // Pin for Indicator

const unsigned int eepromSaveBrightness  = 0;
const unsigned int eepromSaveLightStatus = 1;

const unsigned int onOffDelay             = 24;   // Delay when light on/off
const unsigned int selectBrightnessDelay0 = 100;  // Standart delay beetwen change iteration
const unsigned int selectBrightnessDelay1 = 480;  // Top/Bottom delay beetwen change iteration
const unsigned int offAfterOnDelay        = 1000; // Bounce of contacts delay
const unsigned int buzzerDuration         = 70;   // Duration of buzzer sygnal
const unsigned int buzzerFrequency        = 750;  // Frequency of buzzer sygnal

/*
 * Mode values:
 *  0 - do nothing;
 *  1 - soft on light from low to current brightness;
 *  2 - soft off light from current brightness to low;
 *  3 - soft on light to maximum brightness;
 *  4 - change current brightness;
 */
unsigned int mode = 0;

boolean lightStatus = false; // Status of light (ON/OFF)
boolean byMoving = false;    // Light is On by Moving
// unsigned int brightnessMap[] = {1, 2, 3, 4, 5, 6, 8, 10, 12, 14, 16, 20, 24, 28, 36, 44, 64, 74, 84, 100, 116, 132, 164, 196, 255}; // Current brightness value
unsigned int brightnessMap[] = {40, 48, 56, 65, 75, 87, 98, 110, 122, 134, 147, 160, 174, 189, 201, 221, 239, 258, 283, 317, 355, 395, 439, 486, 534, 585, 640, 697, 759, 825, 900, 1024}; // Current brightness value
unsigned int topBrightness = 31; // Top brightness value (Count element of brightnessMap - 1)
unsigned int currentBrightness;  // Current brightness value
unsigned int selectedBrightness; // Selected brightness value
int dir = 1; // Direction for change brightness

unsigned long currentTime;          // Current Time
unsigned long timeChangeBrightness; // Time when brightness last time was changed
unsigned long onLightTime;          // Time when light is on

AlexButton01 button(pinSensor);

void setup()
{
  EEPROM.begin(8);
  
  Serial.begin(9600);
  pinMode(pinSensor, INPUT_PULLUP);
  pinMode(pinLight, OUTPUT);
  pinMode(pinIndicator, OUTPUT);
  
  button.normal_status = HIGH;

  currentBrightness = 0;
  selectedBrightness = EEPROM.read(eepromSaveBrightness);
  if (selectedBrightness < 0 || selectedBrightness > topBrightness) {
    selectedBrightness = topBrightness;
    EEPROM.write(eepromSaveBrightness, selectedBrightness);
  }
  if (digitalRead(pinAutoOn)) { // Auto on light is allowed
    int autoOn = EEPROM.read(eepromSaveLightStatus);
    if (autoOn < 0 || autoOn > 1) {
      autoOn = 0;
      EEPROM.write(eepromSaveLightStatus, 0);
    }
    if (autoOn == 1) {
      setLight();
      changeMode(1);
    }
  }

  timeChangeBrightness = millis();
}

void loop()
{
  switch (button.check()) {
    case 1: // кнопка нажата
      if (!lightStatus) {
        currentBrightness = 0;
        setLight();
        changeMode(1);
      }
      break;
    case 2: // кнопка нажата дважды
      changeMode(3);
      break;
    case 4: // кнопка удерживается в нажатом состоянии длительное время
      if (mode != 4) {
        changeMode(4);
      }
      break;
    case -1: // кнопка отпущена
      if (mode == 4) {
        dir = dir > 0 ? -1 : 1;
        selectedBrightness = currentBrightness;
        EEPROM.write(eepromSaveBrightness, selectedBrightness);
        changeMode(1);
      } else if (lightStatus && millis() - onLightTime > offAfterOnDelay) {
        changeMode(2);
      } 
      break;
    default:
      break;
  }
  
  if (mode == 0) {
    //Serial.println(digitalRead(pinExtraSensor));
/*
    if (!lightStatus && digitalRead(pinExtraSensor)) {
       byMoving = true;
       changeMode(1);
    } else if (byMoving && lightStatus && !digitalRead(pinExtraSensor)) {
       changeMode(2);
    }
*/    
  }
  
  switch (mode) {
    case 0:
      break;
    case 1:
      softTurnOnLight();
      break;
    case 2:
      softTurnOffLight();
      break;
    case 3:
      setTopBrightness();
      break;
    case 4:
      selectBrightness();
      break;
  }

  Serial.println(digitalRead(pinSensor));
}

void selectBrightness()
{
  unsigned int _delay = (currentBrightness == 0 || currentBrightness == topBrightness) ? selectBrightnessDelay1 : selectBrightnessDelay0;
  if (checkDelay(timeChangeBrightness, _delay)) {
    if (currentBrightness >= topBrightness) {
      currentBrightness = topBrightness;
      dir = -1;
    } else if (currentBrightness <= 0) {
      currentBrightness = 0;
      dir = 1;
    }
    changeCurrentBrightness();
  }
}

void softTurnOnLight()
{
  if (checkDelay(timeChangeBrightness, onOffDelay)) {
    if (currentBrightness < selectedBrightness) {
      dir = 1;
      changeCurrentBrightness();
    }
    if (currentBrightness >= selectedBrightness) {
      currentBrightness = selectedBrightness;
      setLight();
      changeMode(0);
    }
  }
}

void softTurnOffLight()
{
  if (checkDelay(timeChangeBrightness, onOffDelay)) {
    if (currentBrightness > 0) {
      dir = -1;
      changeCurrentBrightness();
    }
    if (currentBrightness <= 0) {
      offLight();
      changeMode(0);
    }
  }
}

void setTopBrightness()
{
  selectedBrightness = topBrightness;
  EEPROM.write(eepromSaveBrightness, selectedBrightness);
  setLight();
  changeMode(1);
}

void changeCurrentBrightness()
{
  currentBrightness += dir;
  setLight();
}

void changeMode(unsigned int _mode)
{
  if (mode != _mode) {
    mode = _mode;
    timeChangeBrightness = millis();
    if (mode == 1) {
      onLightTime = timeChangeBrightness;
    }
    if (mode != 0 && digitalRead(pinAllowBuzzer)) {
      tone(pinBuzzer, buzzerFrequency, buzzerDuration);
    }
  }
}

void setLight()
{
  lightStatus = true;
  EEPROM.write(eepromSaveLightStatus, 1);
  analogWrite(pinLight, brightnessMap[currentBrightness]);
  digitalWrite(pinIndicator, LOW);
}

void offLight()
{
  lightStatus = false;
  EEPROM.write(eepromSaveLightStatus, 0);
  digitalWrite(pinLight, LOW);
  digitalWrite(pinIndicator, HIGH);
  byMoving = false;
}

boolean checkDelay(unsigned long &previousTime, unsigned long delay)
{
  currentTime = millis();
  if (currentTime - previousTime >= delay) {
    previousTime = currentTime;
    return true;
  }
  return false;
}
