#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <SparkFun_TMP117.h>
#include <RTCZero.h>

File myFile;

//TMP117 
TMP117 sensor;
float tempC;

//RTC 
RTCZero rtc;
const byte seconds = 30;
const byte minutes = 52;
const byte hours = 10;

const byte day = 7;
const byte month = 12;
const byte year = 22;

//SD card
const int chipSelect = 4;

void setup ()
{

//RTC setup
  rtc.begin(); // initialize RTC
  rtc.setTime(hours, minutes, seconds);
  rtc.setDate(day, month, year);

//TMP117 setup
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

//SD card setup
  Serial.print("Initializing SD card...");
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    while (1);
  }
  Serial.println("card initialized.");
}
void loop ()
{
  //lecture temperature TMP117
  tempC = sensor.readTempC();
  Serial.println(tempC);
    
  //ouvrir le fichier text dans la carte SD
  File dataFile = SD.open("datalog.txt", FILE_WRITE);

  //écrire les données sur le fichier texte
  if(dataFile) {
    dataFile.print("temperature :");
    dataFile.println(tempC);
    //fermer le fichier
    dataFile.close();
  }
  else {
    Serial.println("error opening datalog.txt");
    while(1);
  }

  delay(2000);
}