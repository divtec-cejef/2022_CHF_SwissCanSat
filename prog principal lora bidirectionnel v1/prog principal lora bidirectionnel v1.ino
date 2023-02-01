#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include "SparkFun_SCD4x_Arduino_Library.h" 
#include <Adafruit_MS8607.h>
#include <Adafruit_Sensor.h>
#include <RTCZero.h>
#include <Arduino_MKRGPS.h>
#include <LoRa.h>

//MS8607 sensor
Adafruit_MS8607 ms8607; 

//SCD41 sensor
SCD4x SCD41; 
int CO2;

//GPS
int latitude;
int longitude;
int altitude;
int speed;
int satellites;

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

//LoRa
int counter = 0;
String outgoing;              // outgoing message
byte msgCount = 0;            // count of outgoing messages
byte localAddress = 0xBB;     // address of this device
byte destination = 0xFF;      // destination to send to
long lastSendTime = 0;        // last send time
int interval = 2000;          // interval between sends

void setup()
{
  //================================================================================
  //SETUP LED
  //================================================================================  
  pinMode(3, OUTPUT);
  
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
  dataFile.println("Date ;heure ;CO2 ;Temperature ;Humidity ;Pressure ;Latitude ;Longitude ;Altitude ;Speed ;Satelittes ");
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
  //================================================================================
  //SETUP LoRa
  //================================================================================
  Serial.begin(9600);                   // initialize serial
  while (!Serial);

  Serial.println("LoRa Duplex with callback");

  if (!LoRa.begin(915E6)) {             // initialize ratio at 915 MHz
    Serial.println("LoRa init failed. Check your connections.");
    while (true);                       // if failed, do nothing
  }

  LoRa.onReceive(onReceive);
  LoRa.receive();
  Serial.println("LoRa init succeeded.");

  //================================================================================
  //SETUP AUTRES COMPOSANTS
  //================================================================================
  digitalWrite(3, HIGH);//led "on"
  pinMode(5, OUTPUT);//buzzer
}


void loop() {

  CO2 = SCD41.getCO2();
  latitude = GPS.latitude();
  longitude = GPS.longitude();
  altitude = GPS.latitude();
  speed = GPS.speed();
  satellites = GPS.satellites();
  sensors_event_t temp, pressure, humidity;
  ms8607.getEvent(&pressure, &temp, &humidity);

  
  if (millis() - lastSendTime > interval) {
    String message = "1";
    sendMessage(message);
    Serial.println("Sending " + message);
    lastSendTime = millis();            // timestamp the message
    interval = 1000;                    // 1 seconds
    LoRa.receive();                     // go back into receive mode
  }

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
      dataFile.print(altitude);
      dataFile.print(" ;");
      dataFile.print(speed);
      dataFile.print(" ;");
      dataFile.println(satellites);
      dataFile.close();
    }
    else {
      Serial.println("error opening datalog.txt");
      while(1);
    }
}


void sendMessage(String outgoing) {
  sensors_event_t temp, pressure, humidity;
  ms8607.getEvent(&pressure, &temp, &humidity);
  LoRa.beginPacket();                   // start packet
  LoRa.write(destination);              // add destination address
  LoRa.write(localAddress);             // add sender address
  LoRa.write(msgCount);                 // add message ID
  LoRa.print(SCD41.getCO2());
  LoRa.print(" |");
  LoRa.print(temp.temperature);
  LoRa.print(" |");
  LoRa.print(humidity.relative_humidity);
  LoRa.print(" |");
  LoRa.print(pressure.pressure);
  LoRa.print(" |");
  LoRa.print(latitude, 7);
  LoRa.print(" |");
  LoRa.print(longitude, 7);
  LoRa.print(" |");
  LoRa.print(altitude);
  LoRa.print(" |");
  LoRa.print(speed);
  LoRa.print(" |");
  LoRa.println(satellites);
  LoRa.endPacket();                     // finish packet and send it
  msgCount++;                           // increment message ID
}


void onReceive(int packetSize) {
  if (packetSize == 0) return;          // if there's no packet, return

  //// read packet header bytes:
  //int recipient = LoRa.read();          // recipient address
  //byte sender = LoRa.read();            // sender address
  //byte incomingMsgId = LoRa.read();     // incoming msg ID
  //byte incomingLength = LoRa.read();    // incoming msg length

  String incoming = "";                 // payload of packet

  while (LoRa.available()) {            // can't use readString() in callback, so
    incoming += (char)LoRa.read();      // add bytes one by one
  }

  if(incoming == "P")
  {
    tone(5, 1000);
    delay(500);
    Serial.println("piazo ON");    
  }  
  
  if(incoming == 'S')
  {
    noTone(5);
    delay(500);
    Serial.println("piazo OFF");
  }

  //// if the recipient isn't this device or broadcast,
  //if (recipient != localAddress && recipient != 0xFF) {
  //  Serial.println("This message is not for me.");
  //  return;                             // skip rest of function
  //}

  // if message is for this device, or broadcast, print details:
  //Serial.println("Received from: 0x" + String(sender, HEX));
  //Serial.println("Sent to: 0x" + String(recipient, HEX));
  //Serial.println("Message ID: " + String(incomingMsgId));
  //Serial.println("Message: " + incoming);
  //Serial.println("RSSI: " + String(LoRa.packetRssi()));
  //Serial.println("Snr: " + String(LoRa.packetSnr()));
  //Serial.println();
}






