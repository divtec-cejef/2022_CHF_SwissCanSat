#include <SPI.h>
#include <LoRa.h>
#include <SD.h>
#include <Wire.h>

//definition des constantes et des variables
//carte SD
File myFile;
const int chipSelect=19;

//LoRa
char lora;
String id;

//Bouton
const int buttonPin=6;
int buttonState=0;
int buttonCount=0;

//setup
void setup() {
  //Port série
  Serial.begin(9600);
  while (!Serial);

  //carte SD
  if(!SD.begin(chipSelect)){
    Serial.println("SD card unavailable");
  }
  myFile = SD.open("data.txt", FILE_WRITE);
  if(myFile){
    myFile.close();
  } else{
    Serial.println("cannot open data.txt");
    
  }

  //LoRa
  if (!LoRa.begin(868E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  
  LoRa.setSpreadingFactor(10);  

  //Bouton
  pinMode(6, INPUT);

  //setup terminé
  Serial.println("setup completed");
}

//loop
void loop() {
  int packetSize = LoRa.parsePacket();
  
  //si il y a des données LoRa
  if (packetSize) {
    //test de l'ID du message
    id=LoRa.read();

    //test si l'ID correspond
    if(id == "124"){
      //afficher que l'ID est valide
      
      //ouvrir le fichier data.txt dans la carte SD
      myFile = SD.open("data.txt", FILE_WRITE);

      //si le fichier s'ouvre correctement
      if(myFile){
        //remettre le premier caractère
        Serial.print("|");
        myFile.print("|");

        //tant qu'il y a des données LoRa
        while (LoRa.available()) {
          lora=LoRa.read();

          //afficher le LoRa dans le Serial
          Serial.print(lora);
          //écrire le LoRa dans la carte SD
          myFile.print(lora);
        }
        Serial.println("RSSI: " + String(LoRa.packetRssi()));        
        //fermer le fichier data.txt
        myFile.close();
      } else{
        //si le fichier data.txt ne s'ouvre pas 
        Serial.println("unable to access data.txt");
      }
    }
  }

  //lire l'état du bouton
  buttonState=digitalRead(buttonPin);

  //si le bouton est pressé et n'étais pas pressé la boucle avant
  if(buttonState==HIGH){
    if(buttonCount == 0){
      //ouvrire le fichier data.txt
      myFile = SD.open("data.txt", FILE_READ);

      //si le fichier s'ouvre correctement
      if(myFile){

        //tant qu'il y a des données dans la carte SD
        while(myFile.available()){

          //écrire les données dans le Serial
          Serial.write(myFile.read());

          //délai pour pas tout casser
          delay(1);
        }
        //fermer le fichier data.txt
        myFile.close();

      } else {
        //si le fichier data.txt ne s'ouvre pas
        Serial.println("unable to access data.txt");
      }
      //le bouton a été pressé      
      buttonCount=1;
    }
  } else {
    //le bouton a été relâché        
    buttonCount=0;
  }
}