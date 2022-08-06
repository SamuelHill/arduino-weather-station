#include <EEPROM.h>
#include <avr/eeprom.h>

typedef struct {
    unsigned long newFileTime;
    char workingFilename[19];
} configuration;

void setup() {
  Serial.begin(9600);
  configuration config = {1550361600L,"/data/19-02-16.csv"};
  eeprom_write_block((const void*)&config, (void*)0, sizeof(config));
  configuration config2;
  eeprom_read_block((void*)&config2, (void*)0, sizeof(config2));
  Serial.print("The value read from EEPROM for newFileTime is: ");
  Serial.println(config2.newFileTime);
  Serial.print("The value read from EEPROM for workingFilename is: ");
  Serial.println(config2.workingFilename);
}

void loop() {}
