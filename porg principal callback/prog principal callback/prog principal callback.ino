//include libraries
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include "SparkFun_SCD4x_Arduino_Library.h" 
#include <Adafruit_MS8607.h>
#include <Adafruit_Sensor.h>
#include <RTCZero.h>
#include <Arduino_MKRGPS.h>
#include <LoRa.h>
#include <math.h>
#include <Arduino_PMIC.h>

//test if the program is compatible with the board used
#ifdef ARDUINO_SAMD_MKRWAN1300
#error "This example is not compatible with the Arduino MKR WAN 1300 board!"
#endif

//define variables
//LoRa
const int csPin = 7;          // LoRa radio chip select
const int resetPin = 6;       // LoRa radio reset
const int irqPin = 1;         // change for your board; must be a hardware interrupt pin

String outgoing;              // outgoing message
byte msgCount = 0;            // count of outgoing messages
byte localAddress = 0xBB;     // address of this device
byte destination = 0xFF;      // destination to send to
long lastSendTime = 0;        // last send time
int interval = 1000;          // interval between sends set to 1 second

//MS8607 sensor
Adafruit_MS8607 ms8607; 

//SCD41 sensor
SCD4x SCD41; 

//SD card
File myFile; 
const int chipSelect = 4;     //SD card on pin 4

//RTC 
RTCZero rtc;
const byte seconds = 30;      //set the second
const byte minutes = 52;      //set the minutes
const byte hours = 10;        //set the hour

const byte day = 12;          //set the day
const byte month = 01;        //set the month
const byte year = 23;         //set the year

//altitude
float po=1034.85;
float k=-0.000126;
float P;
float Alt;
float logOfNumber;
float pressure;

//Pmic boost
int usb_mode = UNKNOWN_MODE;


void setup() {
  Serial.begin(115200);                   // initialize serial

  //================================================================================
  //SETUP Pmic boost
  //================================================================================
  if (!PMIC.begin()) {
    Serial1.println("Failed to initialize PMIC!");
    while (1);
  }

  // Enable boost mode, this mode allows using the board as host to
  // connect a guest device such as a keyboard
  if (!PMIC.enableBoostMode()) {
    Serial1.println("Error enabling Boost Mode");
  }
  Serial1.println("Initialization Done!");
  //Pmic boost
  int actual_mode = PMIC.USBmode();
  if (actual_mode != usb_mode) {
    usb_mode = actual_mode;
    if (actual_mode == BOOST_MODE) {
      // if the boost mode was correctly enabled, 5 V should appear on 5V pin
      // and on the USB connector
      Serial1.println("Boost mode status enabled");
    }
  } 
  delay(2000);

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
  Serial.begin(9600);
  Serial.println(F("SCD4x Example"));
  Wire.begin();

  if (SCD41.begin() == false)
  {
    Serial.println(F("Sensor not detected. Please check wiring. Freezing..."));
    while (1)
      ;
  }
  Serial.println("SCD41 Found!");

  //================================================================================
  //SETUP MS8607
  //================================================================================
  Serial.println("Adafruit MS8607 test!");

  // Try to initialize!
  if (!ms8607.begin()) {
    Serial1.println("Failed to find MS8607 chip");
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
  // If you are using the MKR GPS as shield, change the next line to pass
  // the GPS_MODE_SHIELD parameter to the GPS.begin(...)
  if (!GPS.begin()) {
    Serial.println("Failed to initialize GPS!");
    while (1);
  }
  Serial.println("GPS Found!");

  //================================================================================
  //SETUP LoRa
  //================================================================================
  // override the default CS, reset, and IRQ pins (optional)
  LoRa.setPins(csPin, resetPin, irqPin);// set CS, reset, IRQ pin

  if (!LoRa.begin(868E6)) {             // initialize ratio at 915 MHz
    Serial.println("LoRa init failed. Check your connections.");
    while (true);                       // if failed, do nothing
  }

  LoRa.onReceive(onReceive);
  LoRa.receive();
  Serial.println("LoRa init succeeded.");

  //================================================================================
  //SETUP buzzer/piezo
  //================================================================================
  pinMode(5, OUTPUT);
  Serial.println("init completed");
}

void loop() {
  if (millis() - lastSendTime > interval) {
    //read MS8607 sensor
    sensors_event_t temp, pressure, humidity;
    ms8607.getEvent(&pressure, &temp, &humidity);
    //calculate altitude
    P=pressure.pressure/po;
    logOfNumber = log(P);
    Alt = logOfNumber/k;
    String message = String(rtc.getDay() + '/' + rtc.getMonth() + '/' + rtc.getYear() + ' ' + rtc.getHours() + ':' + rtc.getMinutes() + ':' + rtc.getSeconds() + '|' + SCD41.getCO2() + '|' + temp.temperature + '|' + humidity.relative_humidity + '|' + pressure.pressure + '|' + GPS.latitude() + '|' + GPS.longitude() + '|' + Alt + '|' + GPS.speed() + '|' + GPS.satellites());   // send a message
    sendMessage(message);
    Serial.println("Sending " + message);
    lastSendTime = millis();            // timestamp the message
    interval = 1000;                    // 1 second
    LoRa.receive();                     // go back into receive mode
  }
}

void sendMessage(String message) {
  LoRa.beginPacket();                   // start packet
  LoRa.write(destination);              // add destination address
  LoRa.write(localAddress);             // add sender address
  LoRa.write(msgCount);                 // add message ID
  LoRa.write(message.length());        // add payload length
  LoRa.print(message);                 // add payload
  LoRa.endPacket();                     // finish packet and send it
  msgCount++;                           // increment message ID
}

void onReceive(int packetSize) {
  if (packetSize == 0) return;          // if there's no packet, return

  // read packet header bytes:
  int recipient = LoRa.read();          // recipient address
  byte sender = LoRa.read();            // sender address
  byte incomingMsgId = LoRa.read();     // incoming msg ID
  byte incomingLength = LoRa.read();    // incoming msg length

  char command = 0;                 // payload of packet

  while (LoRa.available()) {            // can't use readString() in callback, so
    command += (char)LoRa.read();      // add bytes one by one
  }

  // if the recipient isn't this device or broadcast,
  if (recipient != localAddress && recipient != 0xFF) {
    Serial.println("This message is not for me.");
    return;                             // skip rest of function
  }

  if(command == 'P'){
    Serial.println("piezo ON");
    tone(5, 1000);
    delay(1);
    command = 0;
  }

  if(command == 'S'){
    Serial.println("piezo OFF");
    noTone(5);
    delay(1);
    command = 0;
  }

  if(command != 'P' && command != 'S'){
    Serial.println("command not valid");
    command = 0;
  }

//  // if message is for this device, or broadcast, print details:
//  Serial.println("Received from: 0x" + String(sender, HEX));
//  Serial.println("Sent to: 0x" + String(recipient, HEX));
//  Serial.println("Message ID: " + String(incomingMsgId));
//  Serial.println("Message length: " + String(incomingLength));
//  Serial.println("Message: " + command);
//  Serial.println("RSSI: " + String(LoRa.packetRssi()));
//  Serial.println("Snr: " + String(LoRa.packetSnr()));
//  Serial.println();
}

