// Example testing sketch for various DHT humidity/temperature sensors
// Written by ladyada, public domain

// REQUIRES the following Arduino libraries:
// - DHT Sensor Library: https://github.com/adafruit/DHT-sensor-library
// - Adafruit Unified Sensor Lib: https://github.com/adafruit/Adafruit_Sensor

#include "DHT.h"
#include <LiquidCrystal.h>


// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 12, en = 11, d4 = 7, d5 = 6, d6 = 5, d7 = 4;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

#define DHTPIN 2     // Digital pin connected to the DHT sensor
// Feather HUZZAH ESP8266 note: use pins 3, 4, 5, 12, 13 or 14 --
// Pin 15 can work but DHT must be disconnected during program upload.

// Uncomment whatever type you're using!
#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

// Connect pin 1 (on the left) of the sensor to +5V
// NOTE: If using a board with 3.3V logic like an Arduino Due connect pin 1
// to 3.3V instead of 5V!
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 3 (on the right) of the sensor to GROUND (if your sensor has 3 pins)
// Connect pin 4 (on the right) of the sensor to GROUND and leave the pin 3 EMPTY (if your sensor has 4 pins)
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

// Initialize DHT sensor.
// Note that older versions of this library took an optional third parameter to
// tweak the timings for faster processors.  This parameter is no longer needed
// as the current DHT reading algorithm adjusts itself to work on faster procs.
DHT dht(DHTPIN, DHTTYPE);

byte customConfidenceIntervalChar[8] = {
  0b00100,
  0b00100,
  0b11111,
  0b00100,
  0b00100,
  0b00000,
  0b11111,
  0b00000
};

byte customCharCentigrade[8] = {
  0b01000,
  0b10100,
  0b01011,
  0b00100,
  0b01000,
  0b01000,
  0b00100,
  0b00011
};

byte customCharWaterDroplet[8] = {
  0b00100,
  0b00100,
  0b01010,
  0b01010,
  0b10001,
  0b10001,
  0b01110,
  0b00000
};


byte customCharThermometer[8] = {
  0b00100,
  0b01010,
  0b01010,
  0b01010,
  0b01010,
  0b10001,
  0b10001,
  0b01110
};

byte customCharFullSquare[8] = {
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111
};

const int SCREEN_WIDTH = 16;

const int SAMPLING_PERIOD = 1000; // ms

const int RAW_READS_WINDOW_SIZE = 8;
// the different measurements done each time we read from the sensor,
// ex : humidity and temperature makes 2
const int MEASUREMENTS_COUNT = 2;

float rawSensorReads[RAW_READS_WINDOW_SIZE][MEASUREMENTS_COUNT];


float sumOfReads[MEASUREMENTS_COUNT];

void initVariables() {

  for (int i = 0; i < RAW_READS_WINDOW_SIZE; ++i) {
    for (int j = 0; j < MEASUREMENTS_COUNT; ++j) {
      rawSensorReads[i][j] = 0.0f;
    }
  }

  for (int j = 0; j < MEASUREMENTS_COUNT; ++j) {
    sumOfReads[j] = 0.0f;
  }
}

void readSensor() {

  for (int j = 0; j < MEASUREMENTS_COUNT; ++j) {
    sumOfReads[j] -= rawSensorReads[0][j];
  }
  for (int i = 0; i < RAW_READS_WINDOW_SIZE - 1; ++i) {
    for (int j = 0; j < MEASUREMENTS_COUNT; ++j) {
      rawSensorReads[i][j] = rawSensorReads[i + 1][j];
    }
  }

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  rawSensorReads[RAW_READS_WINDOW_SIZE - 1][0] = dht.readHumidity();

  // Read temperature as Celsius (the default)
  rawSensorReads[RAW_READS_WINDOW_SIZE - 1][1] = dht.readTemperature();

  // Check if any reads failed and exit early (to try again).
  for (int i = 0; i < MEASUREMENTS_COUNT; ++i) {
    if (isnan(rawSensorReads[RAW_READS_WINDOW_SIZE - 1][i])) {
      lcd.print(F("Failed to read from DHT sensor!"));
      return;
    }
  }

  for (int j = 0; j < MEASUREMENTS_COUNT; ++j) {
    sumOfReads[j] += rawSensorReads[RAW_READS_WINDOW_SIZE - 1][j];
  }
}

void initReads() {
  lcd.setCursor(0, 0);
  lcd.print(F("Initialising"));
  lcd.setCursor(0, 1);

  float progress = 0.0f;

  for (int i = 0; i < RAW_READS_WINDOW_SIZE; ++i) {
    delay(SAMPLING_PERIOD);
    readSensor();
    progress += float(SCREEN_WIDTH + 1) / float(RAW_READS_WINDOW_SIZE);
    while (progress >= 1) {
      lcd.write((byte)4);
      progress -= 1.0f;
    }
  }
}

void setup() {

  initVariables();

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  lcd.createChar(0, customConfidenceIntervalChar);
  lcd.createChar(1, customCharCentigrade);
  lcd.createChar(2, customCharWaterDroplet);
  lcd.createChar(3, customCharThermometer);
  lcd.createChar(4, customCharFullSquare);

  Serial.begin(9600);
  //  Serial.println(F("DHTxx test!"));

  dht.begin();

  initReads();

  lcd.clear();

}

void loop() {

  /******************************************************/
  /*********************  Humidity  *********************/
  /******************************************************/

  float hRaw = sumOfReads[0] / float(RAW_READS_WINDOW_SIZE);
  int h(hRaw);

  lcd.setCursor(0, 0);
  lcd.write((byte)2);
  lcd.print(F("    "));
  lcd.setCursor(2, 0);
  lcd.print(h);
  lcd.setCursor(7, 0);
  lcd.print(F("%  "));
  lcd.write((byte)0);
  lcd.print(" " + String(5));


  /*******************************************************/
  /********************* Temperature *********************/
  /*******************************************************/

  float tRaw = sumOfReads[1] / float(RAW_READS_WINDOW_SIZE);
  String t(float(int(tRaw * 10)) / 10, 1);

  lcd.setCursor(0, 1);
  lcd.write((byte)3);
  lcd.print(F("     "));
  lcd.setCursor(2, 1);
  lcd.print(t);
  lcd.setCursor(7, 1);
  lcd.write((byte)1);
  lcd.print("  ");
  lcd.write((byte)0);
  lcd.print(" " + String(2));

  //------------------------------------------------------
  delay(SAMPLING_PERIOD);
  readSensor();

}
