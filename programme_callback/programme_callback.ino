#define TIMER_INTERRUPT_DEBUG         0
#define _TIMERINTERRUPT_LOGLEVEL_     0
// Select only one to be true for SAMD21. Must must be placed at the beginning before #include "SAMDTimerInterrupt.h"
#define USING_TIMER_TC3         true      // Only TC3 can be used for SAMD51
#define USING_TIMER_TC4         false     // Not to use with Servo library
#define USING_TIMER_TC5         false
#define USING_TIMER_TCC         false
#define USING_TIMER_TCC1        false
#define USING_TIMER_TCC2        false     // Don't use this, can crash on some boards

#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include "SparkFun_SCD4x_Arduino_Library.h" 
#include <Adafruit_MS8607.h>
#include <Adafruit_Sensor.h>
#include <Arduino_MKRGPS.h>
#include <RTCZero.h>
#include <LoRa.h>
#include <math.h>
#include <Arduino_PMIC.h>
#include <SAMDTimerInterrupt.h>
#include <SAMD_ISR_Timer.h>
#include <wdt_samd21.h>

SAMD_ISR_Timer ISR_Timer;
SAMDTimer ITimer(TIMER_TC3);
#define HW_TIMER_INTERVAL_MS      10

//MS8607 sensor
Adafruit_MS8607 ms8607; 

//SCD41 sensor
SCD4x SCD41; 

//SD card
File myFile; 
const int chipSelect = 3;

//GPS 
int temp=0;
int timetosend=0;
float latitude=0;
float longitude=0;
float altitude=0;
float speed=0;
int   satellites=0;

//RTC 
RTCZero rtc;
const byte seconds = 20;
const byte minutes = 13;
const byte hours = 13;

const byte day = 13;
const byte month = 03;
const byte year = 23;

//LoRa
int counter = 0;

//altitude
float po=1008.15;
float k=-0.000126;
float P;
float Alt;
float logOfNumber; 
float pressure;

//Pmic boost
int usb_mode = UNKNOWN_MODE;

//variable bug
int error=0;
#define TIMER_INTERVAL_1S 300L

void TimerHandler(void)
{
  ISR_Timer.run();
}

void erreur()
{

  temp++;
  if(temp>6){
    timetosend=1;    
    temp=0;
  }
  static int count=0;
  switch (error)
  {
    case 0: //No error
      if(count==0)
      {
        digitalWrite(4, 0);
      }
      if(count=3) 
      {
        digitalWrite(4, 1);
      }
      
      if(count==5)
        count=0;

      break;  

    case 1: //error carte sd
      if(count < 1) 
      {
        digitalWrite(4, 0);
      } 
      else if (count < 5)
      {
        digitalWrite(4, !digitalRead(4));        
      }                 

      else
        count=0;      

      break;
    case 2: //error scd41
      if(count < 2) 
      {
        digitalWrite(4, 0);
      } 
      else if (count < 6)
      {
        digitalWrite(4, !digitalRead(4));        
      }                 

      else
        count=0;      

      break;
    case 3: //error ms8607
      if(count < 3) 
      {
        digitalWrite(4, 0);
      } 
      else if (count < 7)
      {
        digitalWrite(4, !digitalRead(4));        
      }                 
      else
        count=0;      
      
      break;
    case 4 : //error gps
      if(count < 4) 
      {
        digitalWrite(4, 0);
      } 
      else if (count < 8)
      {
        digitalWrite(4, !digitalRead(4));        
      }                 

      else
        count=0;

        break;
    case 5: // error LoRa
      if(count < 5) 
      {
        digitalWrite(4, 0);
      } 
      else if (count < 9)
      {
        digitalWrite(4, !digitalRead(4));        
      }                 

      else
        count=0;      

      
      break;
  }
  count++;
}

void setup()
{
  Serial.begin(115200);
  //================================================================================
  //SETUP Pmic boost
  //================================================================================
  
  if (!PMIC.begin()) {
    Serial.println("Failed to initialize PMIC!");
  }

  // Enable boost mode, this mode allows using the board as host to
  // connect a guest device such as a keyboard
  if (!PMIC.enableBoostMode()) {
    Serial.println("Error enabling Boost Mode");
  }
  Serial.println("Initialization Done!");
  //Pmic boost
  int actual_mode = PMIC.USBmode();
  if (actual_mode != usb_mode) {
    usb_mode = actual_mode;
    if (actual_mode == BOOST_MODE) {
      // if the boost mode was correctly enabled, 5 V should appear on 5V pin
      // and on the USB connector
      Serial.println("Boost mode status enabled");
    }
  } 
  delay(2000);

  //================================================================================
  //SETUP LED et buzzer
  //================================================================================  
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);

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
    error=1;
  }
  else{
  Serial.println("card initialized.");
  }
  
  File dataFile = SD.open("datalog.txt", FILE_WRITE);

  //écrire les données sur le fichier texte
  if(dataFile) {
  dataFile.println("Date ;heure ;CO2 ;Temperature ;Humidity ;Pressure ;Latitude ;Longitude ;Altitude ;Speed ;Satelittes ");
  //fermer le fichier
  dataFile.close();
  }
  //================================================================================
  //SETUP SCD41
  //================================================================================
  Serial.println(F("SCD4x Example"));
  Wire.begin();

  if (SCD41.begin() == false)
  {
    Serial.println(F("Sensor not detected. Please check wiring. Freezing..."));
    error=2;
  }
  else{
  Serial.println("SCD41 Found!");
  }  
  //================================================================================
  //SETUP MS8607
  //================================================================================
  Serial.println("Adafruit MS8607 test!");

  // Try to initialize!
  if (!ms8607.begin()) {
    Serial.println("Failed to find MS8607 chip");
    error=3;
  }
  else{
  Serial.println("MS8607 Found!");
  }

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
  //================================================================================
  //SETUP GPS
  //================================================================================
  // If you are using the MKR GPS as shield, change the next line to pass
  // the GPS_MODE_SHIELD parameter to the GPS.begin(...)
  if (!GPS.begin()) {
    Serial.println("Failed to initialize GPS!");
    error=4;
  }
  else{
  Serial.println("GPS Found!");
  }
  //================================================================================
  //SETUP LoRa
  //================================================================================
  Serial.println("LoRa Sender");

  if (!LoRa.begin(868E6)) {
    Serial.println("Starting LoRa failed!");
    error=5;
  }
  else{
    Serial.println("LoRa ok!");
  }

  LoRa.setSpreadingFactor(10);           // ranges from 6-12,default 7 see API docs
  //================================================================================
  //SETUP callback
  //================================================================================
  if (ITimer.attachInterruptInterval_MS(HW_TIMER_INTERVAL_MS, TimerHandler))
    {
      Serial.print(F("Starting ITimer OK, millis() = ")); Serial.println(millis());
    }
    else
      Serial.println(F("Can't set ITimer. Select another freq. or timer"));
  
    // Just to demonstrate, don't use too many ISR Timers if not absolutely necessary
    // You can use up to 16 timer for each ISR_Timer
    ISR_Timer.setInterval(TIMER_INTERVAL_1S,  erreur);
    
  //================================================================================
  //SETUP watchdog
  //================================================================================
   
  wdt_init ( WDT_CONFIG_PER_4K );
}

void loop()
{ 
  if (GPS.available() || timetosend==1) {   
    wdt_reset();
    temp=0;
    timetosend=0;
    latitude   = GPS.latitude();
    longitude  = GPS.longitude();
    altitude   = GPS.altitude();
    speed      = GPS.speed();
    satellites = GPS.satellites(); 
 
    tone (5, 1000); // allume le buzzer actif arduino

    //allumer buzzer
    //if(Alt<500){
    //tone (5, 4000); // allume le buzzer actif arduino
    //}
  
    //print Date & Hour
    Serial.print(rtc.getDay());
    Serial.print("/");
    Serial.print(rtc.getMonth());
    Serial.print("/");    
    Serial.print(rtc.getYear());
    Serial.print(" ");
    Serial.print(rtc.getHours());
    Serial.print(":");     
    Serial.print(rtc.getMinutes()); 
    Serial.print(":");
    Serial.print(rtc.getSeconds());  
    Serial.println();

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

    //calcul altitude
    P=pressure.pressure/po;
    logOfNumber = log(P);
    Alt = logOfNumber/k;
    
    // print GPS values
    Serial.print("Location: ");
    Serial.print(latitude, 7);
    Serial.print(", ");
    Serial.println(longitude, 7);
    Serial.print("Altitude: ");
    Serial.print(Alt);
    Serial.println("m");
    Serial.print("Ground speed: ");
    Serial.print(speed);
    Serial.println(" km/h");
    Serial.print("Number of satellites: ");
    Serial.println(satellites);

    Serial.println();
  
    Serial.print("Sending packet: ");
    Serial.println(counter);

    // send packet
    LoRa.beginPacket();
    
    LoRa.print("|");
    LoRa.print(Alt);
    LoRa.print("|");
    LoRa.print(SCD41.getCO2());
    LoRa.print("|");  
    LoRa.print(temp.temperature); 
    LoRa.print("|");
    LoRa.print(latitude, 7);
    LoRa.print("-");
    LoRa.print(longitude, 7);
    LoRa.print("|");
    LoRa.print(humidity.relative_humidity); 
    LoRa.print("|");
    LoRa.print(pressure.pressure); 
    LoRa.println("|");

    LoRa.endPacket(true);

    counter++;
  
    //write in SD card
    File dataFile = SD.open("datalog.txt", FILE_WRITE);
    if(dataFile) {
      dataFile.print(rtc.getDay());
      dataFile.print("/");
      dataFile.print(rtc.getMonth());
      dataFile.print("/");    
      dataFile.print(rtc.getYear());
      dataFile.print(" ;");
      dataFile.print(rtc.getHours());
      dataFile.print(":");     
      dataFile.print(rtc.getMinutes()); 
      dataFile.print(":");
      dataFile.print(rtc.getSeconds());
      dataFile.print(" ;");
      dataFile.print(SCD41.getCO2());
      dataFile.print(" ;");
      dataFile.print(temp.temperature);
      dataFile.print(" ;");
      dataFile.print(humidity.relative_humidity);
      dataFile.print(" ;");
      dataFile.print(pressure.pressure);
      dataFile.print(" ;");
      dataFile.print(latitude, 7);
      dataFile.print(" ;");
      dataFile.print(longitude, 7);
      dataFile.print(" ;");
      dataFile.print(Alt);
      dataFile.print(" ;");
      dataFile.print(speed);
      dataFile.print(" ;");
      dataFile.println(satellites);
      dataFile.close();
    }
    noTone (5); // allume le buzzer actif arduino
  }     
} 