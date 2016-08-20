/* Weather Station and Soil Moisture Probe Tester
   Reads analog input on A0 from the soil moisture probe.
   Reads analog input on A2 from the photoresistor/light dependant resistor (LDR).
   Reads analog input on A4 as SDA and A5 as SCL from the barometric/temperature pressure sensor (BMP085).
   Reads digital input on pin 6 from the temperature/relative humidity probe (DHT22).
   */

#include <dht.h>
#include <Wire.h>
#include <BMP085.h>

BMP085 dps = BMP085();      // Digital Pressure Sensor 

dht DHT;
int DHT22_PIN = 6; // Digital
int DHT_chk;
//int SoilMPpin = 0; // Analog
//int SoilMPreading;
int LDRpin = 2; // Analog
int LDRreading;
long Temperature = 0, Pressure = 0, Altitude = 0;

// Runs once when you press reset or on power up.
void setup() {
  Serial.begin(9600); // sets serial communication to 9600 bits per second
  Wire.begin();
  delay(1000);
  dps.init(MODE_ULTRA_HIGHRES, 30800, true);
//  analogReference(EXTERNAL); // use AREF for reference voltage (3.3v)
}

void loop(void) {
  LDRreading = analogRead(LDRpin);
//  SoilMPreading = analogRead(SoilMPpin);
  DHT_chk = DHT.read22(DHT22_PIN);
  dps.getPressure(&Pressure);
  dps.getAltitude(&Altitude);
  
  Serial.print("DHT22: ");
  Serial.print(DHT.humidity, 1);
  Serial.print("% Humidity, ");
  Serial.print(DHT.temperature, 1);
  Serial.println(" C");
  Serial.print("Light level: ");
  Serial.println(LDRreading);
//  Serial.print("Soil Moisture: ");
//  Serial.println(SoilMPreading);
  Serial.print("BMP085: ");
  Serial.print("  Alt(cm):");
  Serial.print(Altitude);
  Serial.print("  Temp:");
  dps.calcTrueTemperature();
  Serial.print("  Pressure(Pa):");
  Serial.println(Pressure);
  Serial.println();
  
  delay(2000);
}
