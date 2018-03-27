  /* ESP8266 plus MAX7219 LED Matrix that displays messages revecioved via a WiFi connection using a Web Server
 Provides an automous display of messages
 The MIT License (MIT) Copyright (c) 2017 by David Bird.
 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, but not sub-license and/or 
 to sell copies of the Software or to permit persons to whom the Software is furnished to do so, subject to the following conditions: 
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
See more at http://dsbird.org.uk
*/

//################# LIBRARIES ##########################
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
//#include <WiFiClient.h>
#include <WiFiManager.h>     // https://github.com/tzapu/WiFiManager
#include <DNSServer.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>

int pinCS = D4; // Attach CS to this pin, DIN to MOSI and CLK to SCK (cf http://arduino.cc/en/Reference/SPI )
int numberOfHorizontalDisplays = 4;
int numberOfVerticalDisplays   = 1;
char time_value[20];
String message, message2, webpage;

//################# DISPLAY CONNECTIONS ################
// LED Matrix Pin -> ESP8266 Pin
// Vcc            -> 3v  (3V on NodeMCU 3V3 on WEMOS)
// Gnd            -> Gnd (G on NodeMCU)
// DIN            -> D7  (Same Pin for WEMOS)
// CS             -> D4  (Same Pin for WEMOS)
// CLK            -> D5  (Same Pin for WEMOS)

//################ PROGRAM SETTINGS ####################
String version = "v2.0";       // Version of this program
ESP8266WebServer server(80); // Start server on port 80 (default for a web-browser, change to your requirements, e.g. 8080 if your Router uses port 80
                               // To access server from the outside of a WiFi network e.g. ESP8266WebServer server(8266) add a rule on your Router that forwards a
                               // connection request to http://your_network_ip_address:8266 to port 8266 and view your ESP server from anywhere.
                               // Example http://yourhome.ip:8266 will be directed to http://192.168.0.40:8266 or whatever IP address your router gives to this server
Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);
int wait   = 75; // In milliseconds between scroll movements
int spacer = 1;
int width  = 5 + spacer; // The font width is 5 pixels
String SITE_WIDTH =  "1000";

long previousMillis = 0;        // will store last time Message was updated
 
// the follow variables is a long because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
long interval = 30000;  

void setup() {
  Serial.begin(115200); // initialize serial communications
  WiFiManager wifiManager;
  // New OOB ESP8266 has no Wi-Fi credentials so will connect and not need the next command to be uncommented and compiled in, a used one with incorrect credentials will
  // so restart the ESP8266 and connect your PC to the wireless access point called 'ESP8266_AP' or whatever you call it below in ""
  // wifiManager.resetSettings(); // Command to be included if needed, then connect to http://192.168.4.1/ and follow instructions to make the WiFi connection
  // Set a timeout until configuration is turned off, useful to retry or go to sleep in n-seconds
  wifiManager.setTimeout(180);
  //fetches ssid and password and tries to connect, if connections succeeds it starts an access point with the name called "ESP8266_AP" and waits in a blocking loop for configuration
  if(!wifiManager.autoConnect("SmartLedMessenger")) {
    Serial.println(F("failed to connect and timeout occurred"));
    delay(6000);
    ESP.reset(); //reset and try again
    delay(180000);
  }
  // At this stage the WiFi manager will have successfully connected to a network, or if not will try again in 180-seconds
  Serial.println(F("WiFi connected..."));
  //----------------------------------------------------------------------
  server.begin(); Serial.println(F("Webserver started...")); // Start the webserver  configTime(0 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  matrix.setIntensity(2);    // Use a value between 0 and 15 for brightness
  matrix.setRotation(0, 1);  // The first display is position upside down
  matrix.setRotation(1, 1);  // The first display is position upside down
  matrix.setRotation(2, 1);  // The first display is position upside down
  matrix.setRotation(3, 1);  // The first display is position upside down
  server.on("/", GetMessage);
  wait    = 40;
  message = "Smart Led Messenger (C) 2018";
  display_message(message); // Display the message
  wait    = 75;
  String IPaddress = WiFi.localIP().toString();
  message = "Bienvenue... Configurez moi sur www.smartledmessenger.com";
}

void loop() {
  server.handleClient();
  display_message(message2); // Display the message
  unsigned long currentMillis = millis();
  if(currentMillis - previousMillis > interval) {
    previousMillis = currentMillis; 
    GetServerMessage();
  }
}
void display_message(String message){
   for ( int i = 0 ; i < width * message.length() + matrix.width() - spacer; i++ ) {
    //matrix.fillScreen(LOW);
    int letter = i / width;
    int x = (matrix.width() - 1) - i % width;
    int y = (matrix.height() - 8) / 2; // center the text vertically
    while ( x + width - spacer >= 0 && letter >= 0 ) {
      if ( letter < message.length() ) {    
            matrix.drawChar(x, y, message[letter], HIGH, LOW, 1);// TFT displays the char of ASCII# myOneChar                                         
      }
      letter--;
      x -= width;
    }
    matrix.write(); // Send bitmap to display
    delay(wait/2);
  }
}

void PrepareMessage(String message)
{
  message2 = "";
  for ( int i = 0 ; i < message.length(); i++ ) {   
         uint8_t myOneChar = (uint8_t)  message[i] ; // convert the string char to byte              
         Serial.println(myOneChar);
         if (myOneChar < 128) 
            message2 += message[i];
         else {      
            Serial.println(myOneChar); 
            switch (myOneChar) {
              case 160: // à
                message2 += char(133);                break;
              case 162: // â
                message2 += char(131);                break;
              case 164: // ä
                message2 += char(132);                break;
              case 166: // ae
                message2 += char(145);                break; 
              case 167: // ç
                message2 += char(135);                break;  
              case 168: // è
                message2 += char(138);                break;
              case 169: // é
                message2 += char(130);                break;
              case 170: // ê
                message2 += char(136);                break;
              case 171: // ë
                message2 += char(137);                break;                
              case 174: // î
                message2 += char(140);                break;  
              case 175: // ï
                message2 += char(139);                break;
              case 180: // ô
                message2 += char(147);                break;  
              case 182: // ö
                message2 += char(148);                break;  
              case 187: // û
                message2 += char(150);                break;  
              case 188: // ü
                message2 += char(129);                break;  
              case 185: // ù
                message2 += char(163);                break;  
              case 163: // £
                message2 += char(156);                break;  
              case 194: // °
                message2 += char(248);                break;  
            }                                                                      
        }      
    }  
  Serial.println(message2);
}
void GetServerMessage() {  
    Serial.println("GetServerMessage");
    HTTPClient http;  //Declare an object of class HTTPClient 
    http.begin("http://www.smartledmessenger.com/slm.ashx?key=2");  
    int httpCode = http.GET();                                                                   
    if (httpCode > 0) {  
      message = http.getString();   
      Serial.println(message);     
    }
    http.end(); 
    PrepareMessage(message);
}
void GetMessage() {
  webpage = ""; // don't delete this command, it ensures the server works reliably!
  append_page_header();
  String IPaddress = WiFi.localIP().toString();
  webpage += "<form action=\"http://"+IPaddress+"\" method='post'>";
  webpage += "<P>Enter text for display: <INPUT type='text' size ='60' name='message' value='Enter message'></P></form><br>";
  append_page_footer();
  server.send(200, "text/html", webpage); // Send a response to the client to enter their inputs, if needed, Enter=defaults
  if (server.args() > 0 ) { // Arguments were received
    for ( uint8_t i = 0; i  <= server.args(); i++ ) {
      String Argument_Name   = server.argName(i);
      String client_response = server.arg(i);
      if (Argument_Name == "message") message = client_response;
    }
  }
}

void append_page_header() {
  webpage  = F("<!DOCTYPE HTML><html lang='en'><head>"); // Change language (en) as required
  webpage += "<META HTTP-EQUIV='Content-Type' CONTENT='text/html; charset=utf-8'>";
  webpage += F("<title>Smart Led Messenger</title><style>");
  webpage += F("body {width:");
  webpage += SITE_WIDTH;
  webpage += F("px;margin:0 auto;font-family:arial;font-size:14px;text-align:center;color:#cc66ff;background-color:#F7F2Fd;}");
  webpage += F("ul{list-style-type:circle; margin:0;padding:0;overflow:hidden;background-color:#d8d8d8;font-size:14px;}");
  webpage += F("li{float:left;border-right:1px solid #bbb;}last-child {border-right:none;}");
  webpage += F("li a{display: block;padding:2px 12px;text-decoration:none;}");
  webpage += F("li a:hover{background-color:#FFFFFF;}");
  webpage += F("section {font-size:16px;}");
  webpage += F("p {background-color:#E3D1E2;font-size:16px;}");
  webpage += F("div.header,div.footer{padding:0.5em;color:white;background-color:gray;clear:left;}");
  webpage += F("h1{background-color:#d8d8d8;font-size:26px;}");
  webpage += F("h2{color:#9370DB;font-size:22px;line-height:65%;}");
  webpage += F("h3{color:#9370DB;font-size:16px;line-height:55%;}");
  webpage += F("</style></head><body><h1>Message Display Board ");
  webpage += version+"</h1>";
}

void append_page_footer(){ // Saves repeating many lines of code for HTML page footers
  webpage += F("<ul><li><a href='/'>Enter Message</a></li></ul>");
  webpage += "&copy; Smart Led Messenger";
  webpage += F("</body></html>");
}

