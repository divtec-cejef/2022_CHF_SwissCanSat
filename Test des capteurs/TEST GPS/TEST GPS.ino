#include <Arduino_MKRGPS.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <LoRa.h>

File myFile;

int counter = 0;

const int chipSelect = 4;

void setup() {

//======================================================================
// SETUP GPS
//======================================================================
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

//======================================================================
// SETUP LoRa
//======================================================================

  Serial.begin(9600);
  while (!Serial);

  Serial.println("LoRa Sender");

  if (!LoRa.begin(868E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

//======================================================================
// SETUP SD CARD
//======================================================================
  Serial.print("Initializing SD card...");
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    while (1);
  }
  Serial.println("card initialized.");

  File dataFile = SD.open("datalog.txt", FILE_WRITE);

    //écrire les données sur le fichier texte
    if(dataFile) {
    dataFile.println("latitude;longitude;ALTITUDE;SPEED;SATELITTES");
    //fermer le fichier
    dataFile.close();
  }
  else {
    Serial.println("error opening datalog.txt");
    while(1);
  }

    
}

void loop() {
  // check if there is new GPS data available
  if (GPS.available()) {
    // read GPS values
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

    // send packet
    Serial.print("Sending packet: ");
    Serial.println(counter);

    LoRa.beginPacket();
    LoRa.print("Location: ");
    LoRa.print(latitude, 7);
    LoRa.print(", ");
    LoRa.println(longitude, 7);

    LoRa.print("Altitude: ");
    LoRa.print(altitude);
    LoRa.println("m");

    LoRa.print("Ground speed: ");
    LoRa.print(speed);
    LoRa.println(" km/h");

    LoRa.print("Number of satellites: ");
    LoRa.println(satellites);

    LoRa.println();

    LoRa.endPacket();

    counter++;

  File dataFile = SD.open("datalog.txt", FILE_WRITE);

  //écrire les données sur le fichier texte
    if(dataFile) {
    dataFile.print(latitude, 7);
    dataFile.print(";");
    dataFile.print(longitude, 7);    
    dataFile.print(";");
    dataFile.print(altitude);
    dataFile.print(";");
    dataFile.print(speed);
    dataFile.print(";");
    dataFile.print(satellites);

    dataFile.println();
    //fermer le fichier
    dataFile.close();
  }
  else {
    Serial.println("error opening datalog.txt");
    while(1);
  }
    
    delay(1000);    
  }
}