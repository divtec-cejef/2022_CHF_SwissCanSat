#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <LoRa.h>

File myFile;

char lora;

//SD card
const int chipSelect = 19;

void setup ()
{
  Serial.begin(9600);
//SD card setup
  if (!SD.begin(chipSelect)) {
    while (1);
  }

  //ouvrir le fichier texte
  File dataFile = SD.open("datalog.txt", FILE_WRITE);

  //écrire 'start' sur le fichier texte
  if(dataFile) {
    dataFile.println("start");
    //fermer le fichier
    dataFile.close();
  }
  else {
    while(1);
  }

  //setup LoRa receiver
  while (!Serial);
  if (!LoRa.begin(868E6)) {
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
        lora = LoRa.read();
        //écrire dans le fichier les données LoRa
        dataFile.print(lora);
        Serial.println(lora);
      }      
      //fermer le fichier
      dataFile.close();


    }
    else {
      while(1);
    }
  }

  if(Serial.available())
  {
    String command = Serial.readStringUntil('\n'); // read string until meet newline character

    if(command == "P")
    {
       // turn on LED
      LoRa.beginPacket();
      LoRa.print(command); //send the command to the Cansat
      LoRa.endPacket();

    }
    else
    if(command == "S")
    {
      LoRa.beginPacket();
      LoRa.print(command); //send the command to the Cansat
      LoRa.endPacket();
    }
    
  }
}