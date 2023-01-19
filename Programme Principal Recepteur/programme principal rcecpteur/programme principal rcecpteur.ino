#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <LoRa.h>

File myFile;

//SD card
const int chipSelect = 19;

void setup ()
{
  Serial.begin(9600);

//SD card setup
  Serial.print("Initializing SD card...");
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    while (1);
  }
  Serial.println("card initialized.");

  //ouvrir le fichier texte
  File dataFile = SD.open("datalog.txt", FILE_WRITE);

  //écrire 'start' sur le fichier texte
  if(dataFile) {
    dataFile.println("start");
    //fermer le fichier
    dataFile.close();
  }
  else {
    Serial.println("error opening datalog.txt");
    while(1);
  }

  //setup LoRa receiver
  while (!Serial);

  Serial.println("LoRa Receiver");

  if (!LoRa.begin(868E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
}

void loop ()
{
  // try to parse packet
  int packetSize = LoRa.parsePacket();

  //si on reçoit un packet LoRa
  if(packetSize)
  {

    //ouvrir le fichier text dans la carte SD
    File dataFile = SD.open("datalog.txt", FILE_WRITE);

    //si le fichier s'ouvre correctement
    if(dataFile) {
      //tant qu'on reçoit des données en LoRa
      while(LoRa.available())
      {
        //écrire dans le fichier les données LoRa
        dataFile.print((char)LoRa.read()) && Serial.print((char)LoRa.read());
      }      
      //fermer le fichier
      dataFile.close();


    }
    else {
      //si le fichier ne s'est pas ouvert correctement
      Serial.println("error opening datalog.txt");
      //freeze
      while(1);
    }
  }
}