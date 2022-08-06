#include <LiquidCrystal.h>

#include <myDHT22.h>
#include <Wire.h>
#include <BMP085.h>

BMP085 dps; // Digital Pressure Sensor - takes A4 (SDA) and A5 (SCL)
long Temperature = 0, Pressure = 0;

DHT temp; // Thermometer and humidity gauge
#define DHT22_PIN 5 // Digital

int LDR_OUTPUT;
#define LDR_PIN 2 // Analog

// rs = 9, en = 8, d4 = 7, d5 = 6, d6 = 3, d7 = 2;
LiquidCrystal lcd(9, 8, 7, 6, 3, 2);

void setup(){
  lcd.begin(20, 4);
  Serial.begin(9600);
  Wire.begin();
  delay(1000);
  dps.init(MODE_ULTRA_HIGHRES, 30800, true);
}

void loop()
{
  lcd.clear();
  temp.read22(DHT22_PIN);
  LDR_OUTPUT = analogRead(LDR_PIN);
  dps.calcTrueTemperature();
  dps.getTemperature(&Temperature);
  dps.getPressure(&Pressure);
  
  lcd.setCursor(0,0);
  lcd.print(temp.humidity);
  lcd.print(F("% hum, "));
  lcd.print(temp.temperature);
  lcd.print(F(" C,"));
  lcd.setCursor(0,1);
  lcd.print(LDR_OUTPUT);
  lcd.print(F(" Light level"));
  lcd.setCursor(0,2);
  lcd.print(F("bmp temp: "));
  lcd.print(((Temperature/10.0) * (9.0/5.0)) + 32.0);
  lcd.print(F(" F"));
  lcd.setCursor(0,3);
  lcd.print(F("Pressure: "));
  lcd.print(Pressure/100000.0);
  lcd.print(F(" bar"));
  
  delay(3000);
}
