/*
includes : 
  - sensors : BME280, TMP117, MS8607
  - LoRa communication

missing :
  - SD card storage
  - BMP sensor

*/


//Libraries
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_MS8607.h>
#include <SparkFun_TMP117.h>
#include <LoRa.h>
#include <RTCZero.h>

//BME ports setup
#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

//TMP177 setup
TMP117 sensor;

//MS8607 setup
Adafruit_MS8607 ms8607;

//SD card setup
File myFile;
const int chipSelect = 4;

//RTC setup
RTCZero rtc;
const byte seconds = 30;
const byte minutes = 52;
const byte hours = 10;

const byte day = 7;
const byte month = 12;
const byte year = 22;

//Constants
#define DHTPIN 6     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE); //// Initialize DHT sensor for normal 16mhz Arduino

//Variables
int chk;
float temp2; //temperature BME280
float tempC; //temperature TMP117


//other
Adafruit_BME280 bme;
unsigned long delayTime;
int counter = 0;

void setup()
{
//RTC setup
  rtc.begin(); // initialize RTC
  rtc.setTime(hours, minutes, seconds);
  rtc.setDate(day, month, year);

//BME280 setup
  Serial.println(F("BME280 test"));

  bool status;

  // default settings
  status = bme.begin();  
  if (!status) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }
  
  Serial.println("-- Default Test --");
  delayTime = 1000;

  Serial.println();

//TMP177 setup
  Wire.begin();
  Serial.begin(115200);    // Start serial communication at 115200 baud
  Wire.setClock(400000);   // Set clock speed to be the fastest for better communication (fast mode)
  Serial.println("TMP117 Example 1: Basic Readings");
  if (sensor.begin() == true) // Function to check if the sensor will correctly self-identify with the proper Device ID/Address
  {
    Serial.println("Begin");
  }
  else
  {
    Serial.println("Device failed to setup- Freezing code.");
    while (1); // Runs forever
  }

//MS8607 setup
  Serial.println("Adafruit MS8607 test!");

  // Try to initialize!
  if (!ms8607.begin()) {
    Serial.println("Failed to find MS8607 chip");
    while (1);
  }
  Serial.println("MS8607 Found!");

  ms8607.setHumidityResolution(MS8607_HUMIDITY_RESOLUTION_OSR_8b);
  Serial.print("Humidity resolution set to ");
  switch (ms8607.getHumidityResolution()){
    case MS8607_HUMIDITY_RESOLUTION_OSR_12b: Serial.println("12-bit"); break;
    case MS8607_HUMIDITY_RESOLUTION_OSR_11b: Serial.println("11-bit"); break;
    case MS8607_HUMIDITY_RESOLUTION_OSR_10b: Serial.println("10-bit"); break;
    case MS8607_HUMIDITY_RESOLUTION_OSR_8b: Serial.println("8-bit"); break;
  }
  // ms8607.setPressureResolution(MS8607_PRESSURE_RESOLUTION_OSR_4096);
  Serial.print("Pressure and Temperature resolution set to ");
  switch (ms8607.getPressureResolution()){
    case MS8607_PRESSURE_RESOLUTION_OSR_256: Serial.println("256"); break;
    case MS8607_PRESSURE_RESOLUTION_OSR_512: Serial.println("512"); break;
    case MS8607_PRESSURE_RESOLUTION_OSR_1024: Serial.println("1024"); break;
    case MS8607_PRESSURE_RESOLUTION_OSR_2048: Serial.println("2048"); break;
    case MS8607_PRESSURE_RESOLUTION_OSR_4096: Serial.println("4096"); break;
    case MS8607_PRESSURE_RESOLUTION_OSR_8192: Serial.println("8192"); break;
  }
  Serial.println("");

//SD card setup
  Serial.print("Initializing SD card...");
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    while (1);
  }
  Serial.println("card initialized.");

//setup LoRa
  Serial.println("LoRa Sender");

  if (!LoRa.begin(868E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }  
}

void loop()
{
//LoRa code
  delay(delayTime);


//lecture BME280
  temp2 = bme.readTemperature();

//lecture TMP117
  tempC = sensor.readTempC();

//sending on LoRa
  Serial.print("Sending packet: ");
  Serial.println(counter);
  counter++;
  // send packet
  LoRa.beginPacket();

  
//affichage BME280
  LoRa.println("capteur BME280");
  LoRa.print("Temp: ");
  LoRa.print(temp2);
  LoRa.println(" C");
  LoRa.println();

//afiichage TMP117
  LoRa.println("capteur TMP117");
  LoRa.print("Temp: ");
  LoRa.print(tempC);
  LoRa.println(" C");
  LoRa.println();

//affichage MS8607
  sensors_event_t temp;
  ms8607.getEvent(&pressure);
  LoRa.print("Temperature: ");Serial.print(temp.temperature); Serial.println(" degrees C");
  LoRa.println("");

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
  LoRa.println();
  
  LoRa.println("----------------------------");
  LoRa.println();
  LoRa.endPacket();

//enregistrer les données sur la carte SD
  //créer une chaine poue envoyer les données
  String dataString = "";

  dataString += String(tempC);
  dataString += ", ";

  //ouvrir le fichier text dans la carte SD
  File dataFile = SD.open("datalog.txt", FILE_WRITE);

  //écrire les données sur le fichier texte
  if(dataFile) {
    dataFile.println(dataString);
    //fermer le fichier
    dataFile.close();
  }
  else {
    Serial.println("error opening datalog.txt");
    while(1);
  }

  delay(1000); //Delay 1 sec.
}
