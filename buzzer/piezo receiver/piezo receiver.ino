#include <SPI.h>
#include <Wire.h>
#include <LoRa.h>

char command;
char id;

void setup()
{
  Serial.begin(9600);

  while(!Serial);
  if (!LoRa.begin(868E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  
  pinMode(5, OUTPUT);
  Serial.println("initialisation completed");
}

void loop()
{
// try to parse packet
  int packetSize = LoRa.parsePacket();

  //si on reçoit un packet LoRa
  if(packetSize)
  {  
//    id = LoRa.read();
//    Serial.println("id = " + id);
//    if(id == '5');
//    {
        command = LoRa.read();
        Serial.print("command = ");
        Serial.println(command);      
//    }
  }

  if(command == 'P')
  {
    Serial.println("piazo ON");
    tone(5, 1000);
    delay(500);
    command = 0;
  }

  if(command == 'S')
  {
    Serial.println("piazo ON");
    noTone(5);
    delay(500);
    command = 0;
  }
}