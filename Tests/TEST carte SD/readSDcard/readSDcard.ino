#include <SPI.h>
#include <SD.h>

File myFile;

const int sdCardPin = 19;
const int buttonPin = 6;

int buttonState = 0;
int buttonCount = 0;

void setup() {

  Serial.begin(9600);
  while (!Serial);


  Serial.print("Initializing SD card...");

  if (!SD.begin(sdCardPin)) {
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println("SD card initialized");

  pinMode(buttonPin, INPUT);
  Serial.println("button initialized");
  
  Serial.println("initialization done.");
}

void loop()
{
  buttonState = digitalRead(buttonPin);
  myFile = SD.open("test.txt");
  if(buttonState == HIGH){
    if(buttonCount == 0){
        if (myFile) {
        // read from the file until there's nothing else in it:
        while (myFile.available()) {
          Serial.write(myFile.read());
        }
        // close the file:
        myFile.close();
        buttonCount = 1;        
      } else {
        // if the file didn't open, print an error:
        Serial.println("error opening test.txt");
      }
    }
  } else{
    buttonCount = 0;
  }
}