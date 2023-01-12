#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include "SparkFun_SCD4x_Arduino_Library.h" 
#include <Adafruit_MS8607.h>
#include <Adafruit_Sensor.h>
#include <RTCZero.h>
#include <Arduino_MKRGPS.h>

//MS8607 sensor
Adafruit_MS8607 ms8607; 

//SCD41 sensor
SCD4x SCD41; 

//SD card
File myFile; 
const int chipSelect = 4;

//RTC 
RTCZero rtc;
const byte seconds = 30;
const byte minutes = 52;
const byte hours = 10;

const byte day = 12;
const byte month = 01;
const byte year = 23;

void setup()
{
  //================================================================================
  //SETUP RTC
  //================================================================================
  rtc.begin(); // initialize RTC
  rtc.setTime(hours, minutes, seconds);
  rtc.setDate(day, month, year);

  //================================================================================
  //SETUP SD card
  //================================================================================
  Serial.print("Initializing SD card...");
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    while (1);
  }
  Serial.println("card initialized.");
  
  File dataFile = SD.open("datalog.txt", FILE_WRITE);

  //écrire les données sur le fichier texte
  if(dataFile) {
  dataFile.println("CO2 ;Temperature ;Humidity ;Pressure ;Latitude ;Longitude ;Altitude ;Speed ;Satelittes ");
  //fermer le fichier
  dataFile.close();
  }
  else {
    Serial.println("error opening datalog.txt");
    while(1);
  }

  //================================================================================
  //SETUP SCD41
  //================================================================================
  Serial.begin(115200);
  Serial.println(F("SCD4x Example"));
  Wire.begin();

  if (SCD41.begin() == false)
  {
    Serial.println(F("Sensor not detected. Please check wiring. Freezing..."));
    while (1)
      ;
  }

  //================================================================================
  //SETUP MS8607
  //================================================================================
  Serial.begin(115200);
  while (!Serial) delay(10);     

  Serial.println("Adafruit MS8607 test!");

  // Try to initialize!
  if (!ms8607.begin()) {
    Serial.println("Failed to find MS8607 chip");
    while (1) { delay(10); }
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
  //================================================================================
  //SETUP GPS
  //================================================================================
  // initialize serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // If you are using the MKR GPS as shield, change the next line to pass
  // the GPS_MODE_SHIELD parameter to the GPS.begin(...)
  if (!GPS.begin()) {
    Serial.println("Failed to initialize GPS!");
    while (1);
  }
}

void loop()
{

    // print C02 -SCD41
    Serial.print(F("CO2(ppm):"));
    Serial.print(SCD41.getCO2());
    Serial.println();

    // print HUMIDITY,TEMPERATURE -MS8607   
    sensors_event_t temp, pressure, humidity;
    ms8607.getEvent(&pressure, &temp, &humidity);
    Serial.print("Temperature: ");Serial.print(temp.temperature); Serial.println(" degrees C");
    Serial.print("Humidity: ");Serial.print(humidity.relative_humidity); Serial.println(" %rH");
    Serial.print("Pressure: ");Serial.print(pressure.pressure); Serial.print(" hPa");
    Serial.println("");

    //  print GPS -ASX00017
    float latitude   = GPS.latitude();
    float longitude  = GPS.longitude();
    float altitude   = GPS.altitude();
    float speed      = GPS.speed();
    int   satellites = GPS.satellites();

    // print GPS values
    Serial.print("Location: ");
    Serial.print(latitude, 7);
    Serial.print(", ");
    Serial.println(longitude, 7);

    Serial.print("Altitude: ");
    Serial.print(altitude);
    Serial.println("m");

    Serial.print("Ground speed: ");
    Serial.print(speed);
    Serial.println(" km/h");

    Serial.print("Number of satellites: ");
    Serial.println(satellites);

    Serial.println();
  
    //write in SD card
    File dataFile = SD.open("datalog.txt", FILE_WRITE);
    if(dataFile) {
      dataFile.print(SCD41.getCO2());
      dataFile.print(" ;");
      dataFile.print(temp.temperature);
      dataFile.print(" ;");
      dataFile.print(humidity.relative_humidity);
      dataFile.print(" ;");
      dataFile.print(pressure.pressure);
      dataFile.println(" ;");
      dataFile.print(latitude, 7);
      dataFile.println(" ;");
      dataFile.println(longitude, 7);
      dataFile.println(" ;");
      dataFile.print(altitude);
      dataFile.println(" ;");
      dataFile.print(speed);
      dataFile.println(" ;");
      dataFile.println(satellites);
      dataFile.close();
    }
    else {
      Serial.println("error opening datalog.txt");
      while(1);
    }
    
    delay(1000);
}
