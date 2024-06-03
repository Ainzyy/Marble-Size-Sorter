#include <Arduino.h>

#include <BluetoothSerial.h>
BluetoothSerial SerialBT;

#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C LCD = LiquidCrystal_I2C(0x27, 16, 2);

#define irPin1 19
#define irPin2 18
#define irPin3 5

String message = "";
char incomingChar;

const int irPins[3] = {irPin1, irPin2, irPin3};            // Array to store IR sensor pins (modify pin numbers if needed)
const int numSensors = sizeof(irPins) / sizeof(irPins[0]); // Calculate number of sensors

unsigned long lastDebounceTime[3] = {0}; // Array to store debounce timers for each sensor
const int debounceDelay = 50;

int objectCounts[3] = {0}; // Array to store object counts for each sensor

unsigned long previousMillis = 0;
const int loopDelay = 10; // Adjust loop delay as needed (in milliseconds)

void readIRSensor(int, int);
void displayLCD();

void setup()
{
  Serial.begin(115200);
  SerialBT.begin("Marble Size Sorter");
  for (int i = 0; i < numSensors; i++)
  {
    pinMode(irPins[i], INPUT);
  }
  previousMillis = millis();

  LCD.init();
  LCD.backlight();
  LCD.setCursor(0, 0);
}

void loop()
{
  if (SerialBT.available())
  {
    incomingChar = SerialBT.read();
    if (incomingChar != '\n')
    {
      message += String(incomingChar);
    }
    else
    {
      message = "";
    }
    Serial.write(incomingChar);
  }

  if (message == "Reset")
  {
    for (int i = 0; i < numSensors; i++)
    {
      objectCounts[i] = 0;
    }
    Serial.printf("\nSmall: %d  Medium: %d  Large: %d\n", objectCounts[0], objectCounts[1], objectCounts[2]);
    SerialBT.printf("Small: %d  Medium: %d  Large: %d\n", objectCounts[0], objectCounts[1], objectCounts[2]);
    displayLCD();
  }

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= loopDelay)
  {
    for (int i = 0; i < numSensors; i++)
    {
      readIRSensor(irPins[i], i);
    }
    previousMillis = currentMillis;
  }
}

void readIRSensor(int irPin, int sensorIndex)
{
  unsigned int sensorValue = digitalRead(irPin);
  static int lastSensorValue[3] = {LOW}; // Array to store last sensor values

  if (sensorValue == HIGH && lastSensorValue[sensorIndex] == LOW && (millis() - lastDebounceTime[sensorIndex]) >= debounceDelay)
  {
    objectCounts[sensorIndex]++;
    Serial.printf("Small: %d  Medium: %d  Large: %d\n", objectCounts[0], objectCounts[1], objectCounts[2]);
    SerialBT.printf("Small: %d  Medium: %d  Large: %d\n", objectCounts[0], objectCounts[1], objectCounts[2]);
    displayLCD();
    lastDebounceTime[sensorIndex] = millis();
  }

  lastSensorValue[sensorIndex] = sensorValue;
}

void displayLCD()
{
  LCD.clear();
  LCD.setCursor(0, 0);
  LCD.print("S: ");
  LCD.print(objectCounts[0]);
  LCD.setCursor(0, 1);
  LCD.print("M: ");
  LCD.print(objectCounts[1]);
  LCD.setCursor(9, 0);
  LCD.print("L: ");
  LCD.print(objectCounts[2]);
}