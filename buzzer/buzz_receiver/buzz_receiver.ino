#include <SPI.h>
#include <Wire.h>
#include <LoRa.h>

char command;
char on = 'A';
char off = 'E';

void setup()
{
  Serial.begin(9600);

  while(!Serial);
  if (!LoRa.begin(868E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  Serial.println("initialisation");

pinMode(5, OUTPUT);
}

void loop()
{
// try to parse packet
  int packetSize = LoRa.parsePacket();

  //si on reçoit un packet LoRa
  if(packetSize)
  {  
    while(LoRa.available())
    {
      command = LoRa.read();
      Serial.println(command);
    }
  }
  
  if(command == 'A')
  {
    digitalWrite(5, HIGH);
    Serial.println("LED ON");
  }
  else 
  {
    if(command == 'E')
    {
    digitalWrite(5, LOW);
    Serial.println("LED OFF");
    }
  }
  if(command == 'P')
  {
    tone(5, 1000);
    delay(500);
    Serial.println("piazo ON");
  }

  if(command == 'S')
  {
    noTone(5);
    delay(500);
    Serial.println("piazo OFF");
  }
  command = 0; 
}