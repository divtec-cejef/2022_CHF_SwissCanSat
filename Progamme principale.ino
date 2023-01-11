#include <Wire.h>
#include <SPI.h>
#include "SparkFun_SCD4x_Arduino_Library.h" 
#include <Adafruit_MS8607.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

Adafruit_MS8607 ms8607;
Adafruit_BMP280 bmp;

SCD4x mySensor;

void setup()
{
  //================================================================================
  //SETUP SCD41
  //================================================================================
  Serial.begin(115200);
  Serial.println(F("SCD4x Example"));
  Wire.begin();

  if (mySensor.begin() == false)
  {
    Serial.println(F("Sensor not detected. Please check wiring. Freezing..."));
    while (1)
      ;
  }

  //================================================================================
  //SETUP MS8607
  //================================================================================
  Serial.begin(115200);
  while (!Serial) delay(10);     // will pause Zero, Leonardo, etc until serial console opens

  Serial.println("Adafruit MS8607 test!");

  // Try to initialize!
  if (!ms8607.begin()) {
    Serial.println("Failed to find MS8607 chip");
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
  //SETUP BMP280
  //================================================================================
  Serial.begin(115200);
  while ( !Serial ) delay(100);   // wait for native usb
  Serial.println(F("BMP280 test"));
  unsigned status1;

  status1 = bmp.begin();
  if (!status1) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring or "
                      "try a different address!"));
    Serial.print("SensorID was: 0x"); Serial.println(bmp.sensorID(),16);
    Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
    Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
    Serial.print("        ID of 0x60 represents a BME 280.\n");
    Serial.print("        ID of 0x61 represents a BME 680.\n");
    while (1) delay(10);
  }

  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
}

void loop()
{

    // print C02 -SCD41
    Serial.print(F("CO2(ppm):"));
    Serial.print(mySensor.getCO2());
    Serial.println();

    // print HUMIDITY,TEMPERATURE -MS8607   
    sensors_event_t temp, pressure, humidity;
    ms8607.getEvent(&pressure, &temp, &humidity);
    Serial.print("Temperature: ");Serial.print(temp.temperature); Serial.println(" degrees C");
    Serial.print("Humidity: ");Serial.print(humidity.relative_humidity); Serial.println(" %rH");
    Serial.println("");

    //  print PRESSURE -BMP280
    Serial.print("BMP280: ");
    Serial.print("Pressure = ");
    Serial.print(bmp.readPressure() / 100.0F);
    Serial.println(" hPa");
   
    delay(1000);
}
