/*
 Arduino sketch for interfacing HTML5 canvas display with Arduino

 This Arduino webserver loads HTML5 webpage from SD card
 and updates the canvas element by responding to the 
 AJAX requests made by the webpage. 

 Author : Prabhash K P
 Tested with an Arduino Uno board and Ethernet Shield with Wiznet W5100 chip
*/


#include <SPI.h>
#include <Ethernet.h>
#include <SD.h>

// size of buffer to store HTTP requests
#define REQUEST_BUFFER   50

// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 1, 1); // Change IP address according to your network config
EthernetServer server(80);  // server at port 80
File html5File;             // The HTML5 file on the SD card
char HTTP_requst[50] = {0}; // HTTP request string
int char_cnt = 0;           

void setup()
{
    // disable Ethernet chip at first to read SD card. After this the respective libraries take care
    pinMode(10, OUTPUT);
    digitalWrite(10, HIGH);

    pinMode(0, INPUT);
    pinMode(1, INPUT);
    pinMode(2, INPUT);
    pinMode(3, INPUT);

    Serial.begin(9600);       // debug by serial monitor
    
    // initialize SD card
    Serial.println("Initializing SD card...");
    if (!SD.begin(4)) {
        Serial.println("ERROR - SD card initialization failed!");
        return;    // init failed
    }
    Serial.println("SUCCESS - SD card initialized.");
    // find gauge.htm file
    if (!SD.exists("gauge.htm")) {
        Serial.println("ERROR - Can't find gauge.htm file!");
        return;  // can't find gauge file
    }
    Serial.println("SUCCESS - Found gauge.htm file.");
    
    Ethernet.begin(mac, ip);  // initialize Ethernet device
    server.begin();           // start to listen for clients
}

void loop()
{
    EthernetClient CanvasDisplay = server.available();  // Get CanvasDisplay client

    if (CanvasDisplay) {  // got CanvasDisplay?
        boolean currentLineIsBlank = true; 
        while (CanvasDisplay.connected()) {
            if (CanvasDisplay.available()) {   // check if CanvasDisplay available to read
                char HTTP_requst_char = CanvasDisplay.read(); // read 1 byte (character) from CanvasDisplay request
                    if (char_cnt < (REQUEST_BUFFER - 1)) { //last element is 0 => null terminate string
                    HTTP_requst[char_cnt] = HTTP_requst_char;          // store each HTTP request character in HTTP_requst array
                    char_cnt++;
                }
                if (HTTP_requst_char == '\n' && currentLineIsBlank) { //give response only after last line
                    // send a standard http response header
                    CanvasDisplay.println("HTTP/1.1 200 OK");
                    if (SearchForRequest(HTTP_requst, "ajax_inputs")) {
                        CanvasDisplay.println("Content-Type: text/xml"); // data as XML
                        CanvasDisplay.println("Connection: keep-alive");
                        CanvasDisplay.println();
                        XML_response(CanvasDisplay); //send data to Canvas Display
                    }
                    else{
                        CanvasDisplay.println("Content-Type: text/html");
                        CanvasDisplay.println("Connection: keep-alive");
                        CanvasDisplay.println();
			if (SearchForRequest(HTTP_requst, "GET /digital.htm")) html5File = SD.open("digital.htm"); 
			else if (SearchForRequest(HTTP_requst, "GET /analog.htm")) html5File = SD.open("analog.htm");
			else if (SearchForRequest(HTTP_requst, "GET /bargraph.htm")) html5File = SD.open("bargraph.htm");
			else html5File = SD.open("gauge.htm");
		    }
                     if (html5File) { //if file available in SDcard
                        while(html5File.available()) {
                            CanvasDisplay.write(html5File.read()); // send html5File to Client (CanvasDisplay)
                        }
                        html5File.close();
                    }
                     Serial.print(HTTP_requst);//Debug on serial port
                    // reset buffer
                    char_cnt = 0;
                    for (int i = 0; i < REQUEST_BUFFER; i++) HTTP_requst[i] = 0;
                    break;
                }
                if (HTTP_requst_char == '\n') currentLineIsBlank = true; //new line detected
                else if (HTTP_requst_char != '\r') currentLineIsBlank = false;  //get next character
                
            } // end if (CanvasDisplay.available())
        } // end while (CanvasDisplay.connected())
        delay(1);      // short delay before closing the connection
        CanvasDisplay.stop(); // close the connection
    } // end if (CanvasDisplay)
}

// prepare send the XML file with arduino input values
void XML_response(EthernetClient cd)
{
    int analog_0 = 0;
    int analog_1 = 0;
    int analog_2 = 0;
    int analog_3 = 0;
    int analog_4 = 0;
    int analog_5 = 0;
    int digital_5 = 0;
    int digital_6 = 0;
    int digital_7 = 0;
    int digital_8 = 0;
       
    analog_0 = analogRead(0);
    analog_1 = analogRead(1);
    analog_2 = analogRead(2);
    analog_3 = analogRead(3);
    analog_4 = analogRead(4);
    analog_5 = analogRead(5);
     
    digital_5 = digitalRead(5);
    digital_6 = digitalRead(6);
    digital_7 = digitalRead(7);
    digital_8 = digitalRead(8);
        
    cd.print("<?xml version = \"1.0\" ?>");
    cd.print("<inputs>");

    cd.print("<analog>");
    cd.print(analog_0);
    cd.print("</analog>");
    cd.print("<analog>");
    cd.print(analog_1);
    cd.print("</analog>");
    cd.print("<analog>");
    cd.print(analog_2);
    cd.print("</analog>");
    cd.print("<analog>");
    cd.print(analog_3);
    cd.print("</analog>");
    cd.print("<analog>");
    cd.print(analog_4);
    cd.print("</analog>");
    cd.print("<analog>");
    cd.print(analog_5);
    cd.print("</analog>");

    cd.print("<digital>");
    cd.print(digital_5);
    cd.print("</digital>");
    cd.print("<digital>");
    cd.print(digital_6);
    cd.print("</digital>");
    cd.print("<digital>");
    cd.print(digital_7);
    cd.print("</digital>");
    cd.print("<digital>");
    cd.print(digital_8);
    cd.print("</digital>");
    cd.print("</inputs>");
}


// searches for a needle in the haystack
char SearchForRequest(char *haystack, char *needle)
{
    char needle_index = 0;
    char haystack_index = 0;
    char haystack_length;

    haystack_length = strlen(haystack);
    if (strlen(needle) > haystack_length) return 0;
    while (haystack_index < haystack_length) {
        if (haystack[haystack_index] == needle[needle_index]) {
            needle_index++;
            if (needle_index == strlen(needle)) return 1; // found needle
        }
        else needle_index = 0;
        haystack_index++;
    }
    return 0;
}


