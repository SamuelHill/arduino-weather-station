// Needed to interface with my W5200 ethernet card
#include <SD.h>
#include <SPI.h>
#include <EthernetV2_0.h>
#include <EthernetUdpV2_0.h>
#define W5200_CS 10 // Digital
#define SDCARD_CS 4 // Digital

// Needed for some NPT and formatting
#include <Time.h>
#include <TimeLib.h>
#include <string.h>

// Needed to make the server resiliant to power loss
#include <EEPROM.h>
/*
Could use the library linked here:
  https://gist.github.com/evjrob/88f79dbafea0970ea3faa10685c70687
#include <EEPROMAnything.h>
Instead, use C built in avr. not as pretty but no extra header file
*/
#include <avr/eeprom.h>

// Needed for progmem to clear up some ram space
#include <avr/pgmspace.h>

// Needed for in person display
#include <LiquidCrystal.h>
#define LCD_PIN_1 9
#define LCD_PIN_2 8
#define LCD_PIN_3 7
#define LCD_PIN_4 6
#define LCD_PIN_5 3
#define LCD_PIN_6 2

// Needed for weather station sensors
#include <DHT.h>
#include <Wire.h>
#include <BMP085.h>
#define DHT22_PIN 5 // Digital
#define LDR_PIN 2   // Analog

/****************** ETHERNET STUFF ******************/
byte mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0x4C, 0x64 };
IPAddress ip(192, 168, 1, 177);
EthernetServer server(80);

/******************** NTP STUFF *********************/
unsigned int localPort = 8888;         // local port to listen for UDP packets NIST
IPAddress timeServer(129, 6, 15, 30);  // time server IP address: for more info see
                                       // http://tf.nist.gov/tf-cgi/servers.cgi
const int NTP_PACKET_SIZE= 48;         // NTP time stamp is in the messages first 48 bytes
byte packetBuffer[NTP_PACKET_SIZE];    // buffer to hold incoming and outgoing packets 
EthernetUDP Udp;

/********************* SENSORS **********************/
BMP085 dps = BMP085(); // Digital Pressure Sensor - takes A4 (SDA) and A5 (SCL)
DHT temp;
int DHT_CHK;
int LDR_OUTPUT;
long Temperature = 0, Pressure = 0, Altitude = 0;
LiquidCrystal lcd(LCD_PIN_1, LCD_PIN_2, LCD_PIN_3, LCD_PIN_4, LCD_PIN_5, LCD_PIN_6);

/********************* TIMINGS **********************/
unsigned long lastIntervalTime = 0; // The time the last measurement occured.
#define MEASURE_INTERVAL 60000      // 1 minute intervals between measurements (in ms)
unsigned long newFileTime;          // The time at which we should create a new week's file
#define FILE_INTERVAL 86400         // One day worth of seconds

/********************* CONFIGS **********************/
typedef struct {                     
    unsigned long newFileTime;      //Keeps track of when a newfile should be made.
    char workingFilename[19];       //The path and filename of the current week's file
} configuration;

configuration config;               //Actually make our config struct

/********************* PROGMEM **********************/
// Strings stored in flash mem for the Html Header (saves ram)
const char HeaderOK_0[] PROGMEM = "HTTP/1.1 200 OK";
const char HeaderOK_1[] PROGMEM = "Content-Type: text/html";
const char HeaderOK_2[] PROGMEM = "";

// A table of pointers to the flash memory strings for the header
const char* const HeaderOK_table[] PROGMEM = { HeaderOK_0, HeaderOK_1, HeaderOK_2 };

void HtmlHeaderOK(EthernetClient client) {
    char buffer[30]; // A character array to hold the strings from the flash mem
    for (int i = 0; i < 3; i++) {
        strcpy_P(buffer, (char*)pgm_read_word(&(HeaderOK_table[i]))); 
        client.println( buffer );
    }
} 

// Html 404 Header
const char Header404_0[] PROGMEM = "HTTP/1.1 404 Not Found";
const char Header404_1[] PROGMEM = "Content-Type: text/html";
const char Header404_2[] PROGMEM = "";
const char Header404_3[] PROGMEM = "<h2>File Not Found!</h2>";
const char* const Header404_table[] PROGMEM = { Header404_0, Header404_1,
                                                Header404_2, Header404_3 };

void HtmlHeader404(EthernetClient client) {
    char buffer[30];
    for (int i = 0; i < 4; i++) {
        strcpy_P(buffer, (char*)pgm_read_word(&(Header404_table[i]))); 
        client.println( buffer );
    }
}

/********************** UTILS **********************/
/* A function that creates the index page for when you first connect to 
 *  the arduino. It lists a live page view followed by the files in the
 *  /data/ folder. Make sure this exists on your SD card. */
void ListFiles(EthernetClient client) {
    client.println("<ul>");
    client.print("<li><a href=\"/live.html\">Live</a></li>");
    File workingDir = SD.open("/data/");
    while (true) {
        File entry = workingDir.openNextFile();
        if (! entry) break;
        client.print("<li><a href=\"/graph.html?file=");
        client.print(entry.name());
        client.print("\">");
        client.print(entry.name());
        client.println("</a></li>");
        entry.close();
    }
    workingDir.close();
    client.println("</ul>");
}

// send an NTP request to the time server, necessary for getTime().
unsigned long sendNTPpacket(IPAddress& address) {
    memset(packetBuffer, 0, NTP_PACKET_SIZE);
    packetBuffer[0]  = 0b11100011;
    packetBuffer[1]  = 0;
    packetBuffer[2]  = 6;
    packetBuffer[3]  = 0xEC;
    packetBuffer[12] = 49; 
    packetBuffer[13] = 0x4E;
    packetBuffer[14] = 49;
    packetBuffer[15] = 52;        
    Udp.beginPacket(address, 123);
    Udp.write(packetBuffer,NTP_PACKET_SIZE);
    Udp.endPacket(); 
}

unsigned long getTime() {
    sendNTPpacket(timeServer);
    delay(1000);
    if (Udp.parsePacket()) {
        Udp.read(packetBuffer,NTP_PACKET_SIZE);
        unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
        unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
        unsigned long secsSince1900 = highWord << 16 | lowWord;
        const unsigned long seventyYears = 2208988800UL;
        unsigned long epoch = secsSince1900 - seventyYears;
        return epoch;
    }
}

void getDataFilename(unsigned long rawTime) {
    int yearInt = year(rawTime);
    int monthInt = month(rawTime);
    int dayInt = day(rawTime);
    char newFilename[18] = "";
    char yearStr[5];
    char subYear[3];
    char monthStr[3];
    char dayStr[3];
    strcat(newFilename, "data/");
    itoa(yearInt, yearStr, 10);
    memcpy(subYear, &yearStr[2], 3);
    strcat(newFilename, subYear);
    strcat(newFilename, "-");
    itoa(monthInt, monthStr, 10);
    if (monthInt < 10){
      strcat(newFilename,"0");
    }
    strcat(newFilename,monthStr);
    strcat(newFilename,"-");
    itoa(dayInt,dayStr,10);
    if (dayInt < 10){
        strcat(newFilename,"0");
    }
    strcat(newFilename,dayStr);
    strcat(newFilename,".csv");
    strcpy(config.workingFilename, newFilename);
}

void getDataString(unsigned long rawTime, char *dataString) {
    char timeStr[12];
    ultoa(rawTime, timeStr, 10);
    strncat(dataString, timeStr, strlen(timeStr));
    strcat(dataString,",");
    // Format strings for dht data
    DHT_CHK = temp.read22(DHT22_PIN);
    char tempStr[10];
    dtostrf(temp.temperature, 3, 2, tempStr);
    strcat(dataString,tempStr);
    strcat(dataString,",");
    char humidStr[10];
    dtostrf(temp.humidity, 3, 2, humidStr);
    strcat(dataString,humidStr);
    strcat(dataString,",");
    // Format string for ldr data
    LDR_OUTPUT = analogRead(LDR_PIN);
    char ldrStr[6];
    itoa(LDR_OUTPUT, ldrStr, 10);
    strcat(dataString,ldrStr);
    strcat(dataString,",");
    // Format strings for dps
    dps.calcTrueTemperature();
    dps.getTemperature(&Temperature);
    dps.getPressure(&Pressure);
    dps.getAltitude(&Altitude);
    char altitudeStr[10];
    itoa(Altitude/100, altitudeStr, 10);
    strcat(dataString,altitudeStr);
    strcat(dataString,",");
    char temp2Str[10];
    itoa(Temperature/10, temp2Str, 10);
    strcat(dataString,temp2Str);
    strcat(dataString,",");
    char pressureStr[10];
    itoa(Pressure, pressureStr, 10);
    strcat(dataString,pressureStr);
}

void runLCD() {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(temp.humidity);
    lcd.print("% hum, ");
    lcd.print(temp.temperature);
    lcd.print(" C,");
    lcd.setCursor(0,1);
    lcd.print(LDR_OUTPUT);
    lcd.print(" Light level");
    lcd.setCursor(0,2);
    lcd.print("Alt(m): ");
    lcd.print(Altitude/100);
    lcd.print(", T: ");
    lcd.print(Temperature/10);
    lcd.print(" C");
    lcd.setCursor(0,3);
    lcd.print("Pressure(Pa): ");
    lcd.print(Pressure);
}

void setup() {
    Serial.print("Initializing LCD, Serial, and Wire...");
    lcd.begin(20, 4);
    Serial.begin(9600);
    Wire.begin();
    delay(1000);
    Serial.print("Initializing SD card...");
    pinMode(W5200_CS, OUTPUT);
    digitalWrite(W5200_CS,HIGH);
    pinMode(SDCARD_CS,OUTPUT);
    if (!SD.begin(SDCARD_CS)) {
        Serial.println("initialization failed!");
        return;
    }
    Serial.println("initialization done.");
    Ethernet.begin(mac, ip);
    server.begin();
    Serial.print("server is at ");
    Serial.println(Ethernet.localIP());
    Udp.begin(localPort);
    eeprom_read_block((void*)&config, (void*)0, sizeof(config));
    dps.init(MODE_ULTRA_HIGHRES, 30800, true);
}

#define BUFSIZE 75
#define TRIES 4
#define NIST_SERVER_ERROR 39
// NIST considers retry interval of <4s as DoS attack
#define NIST_DELAY 5000

void loop() {
    if ((millis() % lastIntervalTime) >= MEASURE_INTERVAL) {
        char dataString[20] = "";
        int count = 0;
        unsigned long rawTime;
        rawTime = getTime();
        while ((rawTime == NIST_SERVER_ERROR) && (count < TRIES)) {
            delay(NIST_DELAY);
            rawTime = getTime();
            count += 1;
        }
        if (rawTime != 39){
            if (rawTime >= config.newFileTime){
                getDataFilename(rawTime);
                config.newFileTime += FILE_INTERVAL;
                eeprom_write_block((const void*)&config, (void*)0, sizeof(config));
            }
            getDataString(rawTime, dataString);
            File dataFile = SD.open(config.workingFilename, FILE_WRITE);
            if (dataFile) {
                dataFile.println(dataString);
                dataFile.close();
                Serial.println(dataString);
            }
            else {
                Serial.println("Error opening datafile for writing.");
            }
            digitalWrite(SDCARD_CS,HIGH); // ?
            runLCD(); // relies on getDataString to get sensor values
            lastIntervalTime = millis();
        }
    } else { // Not time for data collection
        char clientline[BUFSIZ];
        int index = 0;
        EthernetClient client = server.available();
        if (client) {
            boolean current_line_is_blank = true;
            index = 0;
            while (client.connected()) {
                if (client.available()) {
                    char c = client.read();
                    if (c != '\n' && c != '\r') {
                        clientline[index] = c;
                        index++;
                        if (index >= BUFSIZ) index = BUFSIZ -1;
                        continue;
                    }
                    clientline[index] = 0;
                    Serial.println(clientline);
                    if (strstr(clientline, "GET / ") != 0) {
                        HtmlHeaderOK(client);
                        client.println("<h2>View data for the day of (yy-mm-dd):</h2>");
                        ListFiles(client);
                    }
                    else if (strstr(clientline, "GET /") != 0) {
                        // this time no space after the /, so a sub-file!
                        char *filename;
                        filename = strtok(clientline + 5, "?");
                        (strstr(clientline, " HTTP"))[0] = 0;
                        Serial.println(filename);
                        File file = SD.open(filename,FILE_READ);
                        if (!file) {
                            HtmlHeader404(client);
                            break;
                        }
                        Serial.println("Opened!");
                        HtmlHeaderOK(client);
                        while(file.available()) {
                            int num_bytes_read;
                            uint8_t byte_buffer[32];
                            num_bytes_read=file.read(byte_buffer,32);
                            client.write(byte_buffer,num_bytes_read);
                        }
                        file.close();
                    }
                    else { HtmlHeader404(client); }
                    break;
                }
            }
            delay(1);
            client.stop();
            digitalWrite(W5200_CS,HIGH); // ?
         }
    }
}
