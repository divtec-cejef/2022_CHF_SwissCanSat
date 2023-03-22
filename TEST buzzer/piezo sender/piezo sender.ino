#include <SPI.h>
#include <LoRa.h>

int counter = 0;
int id = 5;

void setup() {
  Serial.begin(9600);
  pinMode(5, OUTPUT); // set the digital pin as output:

  //setup LoRa
  Serial.begin(9600);
  while (!Serial);

  Serial.println("LoRa Sender");

  if (!LoRa.begin(868E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }  
}

void loop() {
  if(Serial.available()) // if there is data comming
  {
    String command = Serial.readStringUntil('\n'); // read string until meet newline character

    if(command == "P")
    {
       // turn on LED
      LoRa.beginPacket();
//      LoRa.print(id);
      LoRa.print(command);
      LoRa.endPacket();
      Serial.println(command);
    }
    if(command == "S")
    {
       // turn on LED
      LoRa.beginPacket();
//      LoRa.print(id);
      LoRa.print(command);
      LoRa.endPacket();
      Serial.println(command);
    }
  // send packet
  }
}