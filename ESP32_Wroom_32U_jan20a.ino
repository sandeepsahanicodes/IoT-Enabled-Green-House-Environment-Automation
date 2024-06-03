/*
  Sketch generated by the Arduino IoT Cloud Thing "Untitled 2"
  https://create.arduino.cc/cloud/things/7cea995b-2cb1-4813-9b9c-c1c0bd3dbf19

  Arduino IoT Cloud Variables description

  The following variables are automatically generated and updated when changes are made to the Thing

  float humidityValue;
  float temperatureValue;
  int lightIntensityValue;
  int moistureValue;
  bool attackStatus;

  Variables which are marked as READ/WRITE in the Cloud Thing will also have functions
  which are called when their values are changed from the Dashboard.
  These functions are generated with the Thing and added at the end of this sketch.
*/

#include "thingProperties.h"
#include <DHT.h>
#include <DHT_U.h>
#include <gravity_soil_moisture_sensor.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>

#define DHT_SENSOR_PIN 14
#define DHT_TYPE DHT11
#define MOISTURE_SENSOR_PIN 36 // VP Pin on the ESP32 Module.
#define LDR_SENSOR_PIN 39 // VN Pin on the ESP32 Module.

#define COOLING_FAN_PIN 25
#define HEATING_LAMP_PIN 26
#define WATER_PUMP_PIN 27

DHT dht_sensor(DHT_SENSOR_PIN, DHT_TYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);

int sensorMoistureValue, ldrSensorValue;

// Custom Icon for successful network connection.
byte connectedIcon[8] = {
  0b00000,
  0b00000,
  0b00001,
  0b00011,
  0b00111,
  0b01111,
  0b11111,
  0b00000
};

byte emptySpace[8] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000
};

// Custom enum for network type.
enum NetworkStatus {
  CONNECTED,
  DISCONNECTED
} networkStatus = DISCONNECTED;

void setup() {
  peripheralSetup();
  esp32PinModeSetup();

  // Initialize serial and wait for port to open:
  Serial.begin(9600);
  // This delay gives the chance to wait for a Serial Monitor without blocking if none is found
  delay(1500);
  // Defined in thingProperties.h
  initProperties();

  // Connect to Arduino IoT Cloud
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);

  /*
     The following function allows you to obtain more information
     related to the state of network and IoT Cloud connection and errors
     the higher number the more granular information youâll get.
     The default is 0 (only errors).
     Maximum is 4
  */
  ArduinoCloud.addCallback(ArduinoIoTCloudEvent::CONNECT, doThisOnConnect);
  ArduinoCloud.addCallback(ArduinoIoTCloudEvent::DISCONNECT, doThisOnDisconnect);

  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();
  displayAuthors();
}
void loop() {
  ArduinoCloud.update();
  collectDataFromSensors();
  // Validatate collected sensor data.
  if (validateSensorData()) {
    displaySensorValuesOnLCD();
    displayNetworkStatusOnLCD(networkStatus);
  } else {
    displaySensorFailureOnLCD();
    Serial.println("Failed to collect sensor data!");
  }
  delay(1000);
  // Controlling Output peripheral with collected data.
  coolingFanControl();
  heatingLampControl();
  waterPumpControl();

  if (lightIntensityValue < 1024) {
    lcd.init();
    lcd.print("");
    lcd.setCursor(1, 15);
  }
  
}

void esp32PinModeSetup() {
  // Pin setup for DTH11 Module
  pinMode(DHT_SENSOR_PIN, INPUT);

  // Setting following pins as Input.
  pinMode(MOISTURE_SENSOR_PIN, INPUT);
  pinMode(LDR_SENSOR_PIN, INPUT);

  // Pin setup for output devices
  pinMode(COOLING_FAN_PIN, OUTPUT);
  pinMode(HEATING_LAMP_PIN, OUTPUT);
  pinMode(WATER_PUMP_PIN, OUTPUT);
}
void peripheralSetup() {
  // Initialize communication with the DHT11 sensor and ESP32.
  dht_sensor.begin();
  // Setup for LCD Display.
  lcd.init();
  lcd.backlight();
  lcd.clear();
}

void collectDataFromSensors() {
  // Reading DHT11 values.
  temperatureValue = dht_sensor.readTemperature();
  humidityValue = dht_sensor.readHumidity();

  // Reading and converting 12 bit analog moisture value into percentage.
  sensorMoistureValue = analogRead(MOISTURE_SENSOR_PIN);
  moistureValue = int(map(sensorMoistureValue, 0, 4095, 100, 0));

  // Reading and converting 12 bit analog LDR Sensor value into 10 bit value.
  ldrSensorValue = analogRead(LDR_SENSOR_PIN);
  lightIntensityValue = int(map(ldrSensorValue, 0, 4095, 0, 1023));
}
// Gets fired when ESP32 board is connected to cloud.
void doThisOnConnect() {
  Serial.println("Board successfully connected to Arduino IoT Cloud");
  networkStatus = CONNECTED;
}
// Gets fired when ESP32 board is disconnected from cloud.
void doThisOnDisconnect() {
  Serial.println("Board disconnected from Arduino IoT Cloud");
  networkStatus = DISCONNECTED;
}
void displaySensorValuesOnLCD() {
  lcd.setCursor(0, 0);
  lcd.print("T:" + String(temperatureValue, 1) + (char)223 + "C" + " " + "M:" + String(moistureValue) + "%");
  lcd.setCursor(0, 1);
  lcd.print("H:" + String(humidityValue, 1) + "%" + "  " + "LI:" + String(lightIntensityValue));
}
void displaySensorFailureOnLCD() {
  lcd.setCursor(0, 0);
  lcd.print("Failed To");
  lcd.setCursor(0, 1);
  lcd.print("Collect Data!");
  displayNetworkStatusOnLCD(networkStatus);
}
void displayNetworkStatusOnLCD(NetworkStatus status) {
  switch (status) {
    case CONNECTED:
      lcd.createChar(2, connectedIcon);
      lcd.setCursor(15, 0);
      lcd.write(byte(2));
      break;

    case DISCONNECTED:
      lcd.createChar(2, connectedIcon);
      lcd.setCursor(15, 0);
      lcd.write(byte(2));
      delay(500);
      lcd.createChar(2, emptySpace);
      lcd.write(byte(2));
      break;
  }
}
void displayAuthors() {
  lcd.begin(16, 2); 
  lcd.print("Project by:");
  lcd.setCursor(0, 1);
  lcd.print("Sandeep & Subham"); // Display project authors
  delay(2000); // Wait for 2 seconds before continuing
  lcd.clear(); // Clear the LCD
  lcd.print("CSE Batch:");
  lcd.setCursor(0, 1);
  lcd.print("2020-2024"); 
  delay(2000); // Wait for 2 seconds before continuing
  lcd.clear(); // Clear the LCD
}
bool validateSensorData() {
  if (isnan(humidityValue) || isnan(temperatureValue) || isnan(moistureValue) || isnan(lightIntensityValue)) {
    return false;
  } else {
    return true;
  }
}
void coolingFanControl() {
  if (temperatureValue > 30.0 || humidityValue > 70.0 || moistureValue < 70) {
    digitalWrite(COOLING_FAN_PIN, HIGH);
  } else {
    digitalWrite(COOLING_FAN_PIN, LOW);
  }
}
void heatingLampControl() {
  if (temperatureValue < 20.0 || lightIntensityValue < 400) {
    digitalWrite(HEATING_LAMP_PIN, HIGH);
  } else {
    digitalWrite(HEATING_LAMP_PIN, LOW);
  }
}
void waterPumpControl() {
  if (humidityValue < 65.0 || moistureValue < 70.0) {
    digitalWrite(WATER_PUMP_PIN, HIGH);
  } else {
    digitalWrite(WATER_PUMP_PIN, LOW);
  }
}

void onAttackStatusChange()  {
  // Add your code here to act upon AttackStatus change
}