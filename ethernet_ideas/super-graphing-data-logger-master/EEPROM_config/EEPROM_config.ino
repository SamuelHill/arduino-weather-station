/* ************************************************************************
 * ***            Super Graphing Data Logger - EEPROM config            ***
 * ************************************************************************
 * Everett Robinson, December 2016.
 *
 * The following extra non standard libraries were used, and will need to be
 * added to the libraries folder:
 * - EEPROMAnything: http://playground.arduino.cc/Code/EEPROMWriteAnything
 *
 * This sketch helps you set the values in EEPROM which are necessary for
 * Super Graphing Data Logger. It should only need the be run once before
 * the first time you set up SGDL, or in the unlikely event that the EEPROM
 * becomes corrupted.
 *
 * Please ensure that the values in configuration config are appropriate for
 * your project before uncommenting the EEPROM_writeAnything(0, config); line.
 *
 */

#include <EEPROM.h>
#include <EEPROMAnything.h>

typedef struct{
    unsigned long newFileTime;
    char workingFilename[19];
  } configuration;

//This is a one off thing, so everything is in setup
void setup(){
  Serial.begin(9600);
  
  //Create the config struct to write to EEPROM, change values as appropriate
  //Make sure your filename is not too long for the workingFilename char array 
  configuration config = {1356912000L,"/data/25-12-12.csv"};
  //Write the values to the EEPROM
  //EEPROM_writeAnything(0, config);       //Uncomment when you're sure everything is correct
  configuration config2;                   //Create a second config struct for verification
  EEPROM_readAnything(0,config2);
  Serial.print("The value read from EEPROM for newFileTime is: ");
  Serial.println(config2.newFileTime);
  Serial.print("The value read from EEPROM for workingFilename is: ");
  Serial.println(config2.workingFilename);
  Serial.println("If those values are correct then everything went as planned. Otherwise,");
  Serial.println("please double check that the values declared for the struct config are");
  Serial.println("correct and that that EEPROM_writeAnything line is uncommented.");
}


void loop(){
}
