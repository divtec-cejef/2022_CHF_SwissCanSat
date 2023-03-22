//librairies
#include <Arduino.h>
#include <SensirionI2CScd4x.h>
#include <Wire.h>
#include <Adafruit_BME280.h>
#include <LoRa.h>
#include <RTCZero.h>
#include <Adafruit_MS8607.h>
#include <SD.h>
#include <SPI.h>

//BME ports setup
Adafruit_BME280 bme;
#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

//MS8607 setup
Adafruit_MS8607 ms8607;

//SCD41 setup
SensirionI2CScd4x scd4x;

void printUint16Hex(uint16_t value) {
  Serial.print(value < 4096 ? "0" : "");
  Serial.print(value < 256 ? "0" : "");
  Serial.print(value < 16 ? "0" : "");
  Serial.print(value, HEX);
}

void printSerialNumber(uint16_t serial0, uint16_t serial1, uint16_t serial2) {
  Serial.print("Serial: 0x");
  printUint16Hex(serial0);
  printUint16Hex(serial1);
  printUint16Hex(serial2);
  Serial.println();
}


//SD card setup
File myFile;
const int chipSelect = 4;

//RTC setup
RTCZero rtc;
const byte seconds = 40;
const byte minutes = 35;
const byte hours = 13;

const byte day = 14;
const byte month = 12;
const byte year = 22;

//Variables

float Hum1;  //humidité BME280
float Hum2;  //humidité MS8607
float Hum4;  //humidité SCD41




void setup() {
  // put your setup code here, to run once:
  //RTC setup
  rtc.begin();  // initialize RTC
  rtc.setTime(hours, minutes, seconds);
  rtc.setDate(day, month, year);

  //BME280 setup
  Serial.println(F("BME280 test"));
  bool status;
  // default settings
  status = bme.begin();
  if (!status) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1)
      ;
  }

  //MS8607 setup
  Serial.println("Adafruit MS8607 test!");

  // Try to initialize!
  if (!ms8607.begin()) {
    Serial.println("Failed to find MS8607 chip");
    while (1)
      ;
  }
  Serial.println("MS8607 Found!");

  ms8607.setHumidityResolution(MS8607_HUMIDITY_RESOLUTION_OSR_8b);
  Serial.print("Humidity resolution set to ");
  switch (ms8607.getHumidityResolution()) {
    case MS8607_HUMIDITY_RESOLUTION_OSR_12b: Serial.println("12-bit"); break;
    case MS8607_HUMIDITY_RESOLUTION_OSR_11b: Serial.println("11-bit"); break;
    case MS8607_HUMIDITY_RESOLUTION_OSR_10b: Serial.println("10-bit"); break;
    case MS8607_HUMIDITY_RESOLUTION_OSR_8b: Serial.println("8-bit"); break;
  }
  // ms8607.setPressureResolution(MS8607_PRESSURE_RESOLUTION_OSR_4096);
  Serial.print("Pressure and Temperature resolution set to ");
  switch (ms8607.getPressureResolution()) {
    case MS8607_PRESSURE_RESOLUTION_OSR_256: Serial.println("256"); break;
    case MS8607_PRESSURE_RESOLUTION_OSR_512: Serial.println("512"); break;
    case MS8607_PRESSURE_RESOLUTION_OSR_1024: Serial.println("1024"); break;
    case MS8607_PRESSURE_RESOLUTION_OSR_2048: Serial.println("2048"); break;
    case MS8607_PRESSURE_RESOLUTION_OSR_4096: Serial.println("4096"); break;
    case MS8607_PRESSURE_RESOLUTION_OSR_8192: Serial.println("8192"); break;
  }
  Serial.println();

  //SCD41 setup
  uint16_t error;
  char errorMessage[256];

  scd4x.begin(Wire);

  // stop potentially previously started measurement
  error = scd4x.stopPeriodicMeasurement();
  if (error) {
    Serial.print("Error trying to execute stopPeriodicMeasurement(): ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
  }

  uint16_t serial0;
  uint16_t serial1;
  uint16_t serial2;
  error = scd4x.getSerialNumber(serial0, serial1, serial2);
  if (error) {
    Serial.print("Error trying to execute getSerialNumber(): ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
  } else {
    printSerialNumber(serial0, serial1, serial2);
  }

  // Start Measurement
  error = scd4x.startPeriodicMeasurement();
  if (error) {
    Serial.print("Error trying to execute startPeriodicMeasurement(): ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
  }


  //SD card setup
  Serial.print("Initializing SD card...");
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    while (1)
      ;
  }
  Serial.println("card initialized.");

  //setup LoRa
  Serial.println("LoRa Sender");

  if (!LoRa.begin(868E6)) {
    Serial.println("Starting LoRa failed!");
    while (1)
      ;
  }

  //écrire l'ordre des capteur dans la carte SD
  //ouvrir le fichier text dans la carte SD
  File dataFile = SD.open("datalog.txt", FILE_WRITE);
  if (dataFile) {
    dataFile.print("date et heure; BME280; MS8607; STH31; SCD41;");
    dataFile.println();
    //fermer le fichier
    dataFile.close();
  } else {  //erreur si le fichier ne s'ouvre pas
    Serial.println("error opening datalog.txt");
    while (1)
      ;
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  //BME280 lecture
  Hum1 = bme.readHumidity();

  //MS8607 lecture
  sensors_event_t temp, pressure, humidity1;
  ms8607.getEvent(&pressure, &temp, &humidity1);
  Hum2 = humidity1.relative_humidity;

  //SCD41 lecture
  uint16_t error;
  char errorMessage[256];

  // Read Measurement
  uint16_t co2;
  float temperature;

  error = scd4x.readMeasurement(co2, temperature, Hum4);
  if (error) {
    Serial.print("Error trying to execute readMeasurement(): ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
  } else if (co2 == 0) {
    Serial.println("Invalid sample detected, skipping.");
  } else {
  }

  //affichage sur LoRa
  LoRa.beginPacket();
  //BME280
  LoRa.println("capteur BME280");
  LoRa.print("Hum: ");
  LoRa.print(Hum1);
  LoRa.println(" %");
  LoRa.println();

  //MS8607
  LoRa.println("capteur MS8607");
  LoRa.print("Hum: ");
  LoRa.print(Hum2);
  LoRa.println(" %");
  LoRa.println();
  
  //SCD41
  LoRa.println("capteur SCD41");
  LoRa.print("Hum: ");
  LoRa.print(Hum4);
  LoRa.println(" %");
  LoRa.println();

  //affichage date et heure
  LoRa.print(rtc.getDay());
  LoRa.print("/");
  LoRa.print(rtc.getMonth());
  LoRa.print("/");
  LoRa.print(rtc.getYear());
  LoRa.print(" ");
  LoRa.print(rtc.getHours());
  LoRa.print(":");
  LoRa.print(rtc.getMinutes());
  LoRa.print(":");
  LoRa.print(rtc.getSeconds());
  LoRa.println();

  LoRa.println("----------------------------");
  LoRa.println();
  LoRa.endPacket();
  //enregistrer les données sur la carte SD

  //ouvrir le fichier text dans la carte SD
  File dataFile = SD.open("datalog.txt", FILE_WRITE);

  //trester si on peut ouvrir le fichier texte
  if (dataFile) {
    //écrire les données dans le fichier texte
    //affichage date et heure
    dataFile.print(rtc.getDay());
    dataFile.print("/");
    dataFile.print(rtc.getMonth());
    dataFile.print("/");
    dataFile.print(rtc.getYear());
    dataFile.print(" ");
    dataFile.print(rtc.getHours());
    dataFile.print(":");
    dataFile.print(rtc.getMinutes());
    dataFile.print(":");
    dataFile.print(rtc.getSeconds());
    dataFile.print(";  ;");

    //BME280
    dataFile.print(Hum1);
    dataFile.print(";");
    //MS8607
    dataFile.print(Hum2);
    dataFile.print(";");

    //SCD41
    dataFile.print(Hum4);
    dataFile.print(";");
    dataFile.println();

    //fermer le fichier
    dataFile.close();
  } else {  //erreur si le fichier ne s'ouvre pas
    Serial.println("error opening datalog.txt");
    while (1)
      ;
  }
  delay(1000);
}
