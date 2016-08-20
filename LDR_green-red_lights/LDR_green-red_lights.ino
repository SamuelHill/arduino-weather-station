/* Weather Station and Soil Moisture Probe Tester
   Reads analog input on A0 from the soil moisture probe.
   Reads analog input on A2 from the photoresistor/light dependant resistor (LDR).
   Reads analog input on A4 as SDA and A5 as SCL from the barometric/temperature pressure sensor (BMP085).
   Reads digital input on pin 6 from the temperature/relative humidity probe (DHT22).
   */

#include <dht.h>

dht DHT;

int DHT22_PIN = 6; // Digital
int DHT_chk;
int SoilMPpin = 0; // Analog
int SoilMPreading;
int LDRpin = 2; // Analog
int LDRreading;

int LED_red = 12;
int LED_green = 13;

// Runs once when you press reset or on power up.
void setup() {
  Serial.begin(9600); // sets serial communication to 9600 bits per second
  analogReference(EXTERNAL); // use AREF for reference voltage (3.3v)
  
  pinMode(LED_red, OUTPUT);
  pinMode(LED_green, OUTPUT);
}

void loop(void) {
  LDRreading = analogRead(LDRpin);
  //SoilMPreading = analogRead(SoilMPpin);
  //DHT_chk = DHT.read22(DHT22_PIN);
  
  if (LDRreading < 100) {
    digitalWrite(LED_red, HIGH);
    digitalWrite(LED_green, LOW);
  } else {
    digitalWrite(LED_red, LOW);
    digitalWrite(LED_green, HIGH);
  }
  delay(250);
}
