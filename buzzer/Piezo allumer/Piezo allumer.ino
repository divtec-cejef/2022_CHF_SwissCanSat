#include <SPI.h>
#include <LoRa.h>

int counter = 0;

void setup() {
  Serial.begin(9600);
  pinMode(5, OUTPUT); // set the digital pin as output:

  //setup LoRa
  Serial.begin(9600);
  while (!Serial);

  Serial.println("LoRa Sender");

  if (!LoRa.begin(868E5)) {
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
      Serial.println("piezo allumé"); // send action to Serial Monitor
      LoRa.beginPacket();
      LoRa.print(command);
      LoRa.endPacket();

    }
    else
    if(command == "S")
    {
      Serial.println("piezo éteint"); // send action to Serial Monitor
      LoRa.beginPacket();
      LoRa.print(command);
      LoRa.endPacket();
    }
  Serial.print("Sending packet: ");
  Serial.println(counter);

  // send packet

  counter++;
    
  }
}