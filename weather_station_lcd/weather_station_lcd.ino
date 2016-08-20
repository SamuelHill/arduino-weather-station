#include <LiquidCrystal.h>

#include <dht.h>
#include <Wire.h>
#include <BMP085.h>

BMP085 dps = BMP085();      // Digital Pressure Sensor 
//A4 (SDA), A5 (SCL)
dht DHT;
int DHT22_PIN = 6; // Digital
int DHT_chk;
//int SoilMPpin = 0; // Analog
//int SoilMPreading;
int LDRpin = 2; // Analog
int LDRreading;
long Temperature = 0, Pressure = 0, Altitude = 0;

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

void setup(){
  // set up the LCD's number of columns and rows: 
  lcd.begin(20, 4);
  // initialize the serial communications:
  Serial.begin(9600); // sets serial communication to 9600 bits per second
  Wire.begin();
  delay(1000);
  dps.init(MODE_ULTRA_HIGHRES, 30800, true);
}

void loop()
{
  lcd.clear();
  LDRreading = analogRead(LDRpin);
//  SoilMPreading = analogRead(SoilMPpin);
  DHT_chk = DHT.read22(DHT22_PIN);
  dps.calcTrueTemperature();
  dps.getTemperature(&Temperature);
  dps.getPressure(&Pressure);
  dps.getAltitude(&Altitude);
  
  lcd.setCursor(0,0);
  lcd.print(DHT.humidity);
  lcd.print("% hum, ");
  lcd.print(DHT.temperature);
  lcd.print(" C,");
  lcd.setCursor(0,1);
  lcd.print(LDRreading);
  lcd.print(" Light level");
//  Serial.print("Soil Moisture: ");
//  Serial.println(SoilMPreading);
  lcd.setCursor(0,2);
  lcd.print("Alt(m): ");
  lcd.print(Altitude/100);
  lcd.print(", T: ");
  lcd.print(Temperature/10);
  lcd.print(" C");
  lcd.setCursor(0,3);
  lcd.print("Pressure(Pa): ");
  lcd.print(Pressure);
  
  delay(2000);
}
