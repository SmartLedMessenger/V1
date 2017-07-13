/* Project: RGB matrix displaying temperature, humidity, and pressure
 *  using DHT11 & BMP180 sensors and Arduino Uno
 * 
 * Authors: Raj Bhatt (www.embedded-lab.com), Raphaël Maginot (www.smartledmessenger.com)
 * 
 */

#include <Adafruit_GFX.h>   // Core graphics library
#include <RGBmatrixPanel.h> // Hardware-specific library
#include <Wire.h>
#include "RTClib.h"

#define CLK 8  // MUST be on PORTB! (Use pin 11 on Mega)
#define LAT A3
#define OE  9
#define A   A0
#define B   A1
#define C   A2

#include <SFE_BMP180.h>
SFE_BMP180 pressure;

#define DHTPIN 12     // what pin we're connected to

#include <DHT.h>
DHT dht(DHTPIN, DHT11);

#define F2(progmem_ptr) (const __FlashStringHelper *)progmem_ptr

RGBmatrixPanel matrix(A, B, C, CLK, LAT, OE, false);

RTC_DS1307 rtc;
char *daysOfTheWeek[] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
char *monthsOfyear[] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};
int h;
double t, p;
byte red, green, blue, BackgroundBalls=1;
int hue1 = 0, hue2 = 1536;
long j=0;
String message="";
//const char message[] PROGMEM = "";
int    textX   = matrix.width(),
       textMin = sizeof(message) * -12,
       hue     = 0;
int8_t ball[3][4] = {
  {  3,  0,  1,  1 }, // Initial X,Y pos & velocity for 3 bouncy balls
  { 17, 15,  1, -1 },
  { 27,  4, -1,  1 }
};
static const uint16_t PROGMEM ballcolor[3] = {
  0x0080, // Green=1
  0x0002, // Blue=1
  0x1000  // Red=1
};

void Display_Data(){
  
  matrix.setTextSize(1);
  matrix.fillScreen(0);
  
   // Thermo-Hygro
  h = dht.readHumidity();
  //h=10;
  //Serial.println(h);
  // Barometer
  char status;
  status = pressure.startTemperature();
  if (status != 0)
  {
    delay(status);
    pressure.getTemperature(t);
  }
  status = pressure.startPressure(3);
  if (status != 0)
  {
    delay(status);
    status = pressure.getPressure(p, t);
  }
  Serial.println(p);
// Display temperature, humidity, pressure
  matrix.setTextColor(matrix.Color333(7, 0, 0));  // Red color
  matrix.setCursor(0, 0);   // start at top left
  matrix.print((int)t);
  matrix.print((char)248);  // Degree symbol
  matrix.print('C');
  float tf = t*1.8+32;
  matrix.setCursor(0, 9);   // start at second row left
  matrix.setTextColor(matrix.Color333(7, 5, 0));
  matrix.print((int)tf);
  matrix.print((char)248);  // Degree symbol
  matrix.print('F');
  delay(3000);
  matrix.fillScreen(0);
  matrix.setCursor(0, 0);
  matrix.setTextColor(matrix.Color333(0, 7, 0));
  matrix.print("Hum=");
  matrix.setCursor(0, 9);   // start at second row left
  matrix.print(h,1);
  matrix.print('%');
  delay(3000);
  matrix.fillScreen(0);
  matrix.setCursor(0, 0);
  matrix.setTextColor(matrix.Color333(3, 3, 7));
  matrix.print(p);
  matrix.setCursor(9, 9);
  matrix.print("hPa");
  delay(3000);
  

//Display time
   DateTime now = rtc.now();
   matrix.fillScreen(0);
   matrix.setCursor(2, 0);   
   matrix.setTextColor(matrix.Color333(5, 5, 5));
   if(now.hour()<10) matrix.print(0, DEC);
   matrix.print(now.hour()+2, DEC);
   matrix.print(':');
   if(now.minute()<10) matrix.print(0, DEC); 
   matrix.print(now.minute(), DEC);
   matrix.setCursor(7, 9);
   matrix.setTextColor(matrix.Color333(2, 5, 7));
   matrix.print(daysOfTheWeek[now.dayOfTheWeek()]);
  
   delay(3000);
   
  //DateTime now = rtc.now();
    matrix.fillScreen(0);
    matrix.setCursor(2, 0);   
    matrix.setTextColor(matrix.Color333(4, 4, 7));
    if(now.month()<10) matrix.print(0, DEC);
    matrix.print(now.month(), DEC);
    matrix.print('/');
    matrix.print(now.day(), DEC);
    matrix.setCursor(4, 9);
    matrix.setTextColor(matrix.Color333(4, 4, 7));
    matrix.print(now.year(), DEC);
    delay(3000);
  
  //Construct String message
  message = monthsOfyear[now.month()-1];
  message +=' ';
  message += now.day();
  message += ", ";
  message += now.year();
  message += ", ";
  message += daysOfTheWeek[now.dayOfTheWeek()];
  message += " ";
  message += now.hour()+2;
  message += ':';
  message += now.minute();
  message +="  T=";
  message += t;
  message += ((char)248);
  message +="C, Humidity=";
  message += h;
  message +="% ";
  
  textMin = message.length()* -12;
  Serial.print("Message::");
  Serial.println(message);
}

void Scroll_Text(){
  
  byte n = matrix.width();
  textX = matrix.width();
  byte x = message.length();
  Serial.print("x");
  Serial.println(x);
  int loop = 2*(x*12+32);
  while(j<loop){
  byte i;

  // Clear background
  matrix.fillScreen(0);

  // Bounce three balls around
  if(BackgroundBalls){
    for(i=0; i<3; i++) {
      // Draw 'ball'
      matrix.fillCircle(ball[i][0], ball[i][1], 5, pgm_read_word(&ballcolor[i]));
      // Update X, Y position
      ball[i][0] += ball[i][2];
      ball[i][1] += ball[i][3];
      // Bounce off edges
      if((ball[i][0] == 0) || (ball[i][0] == (matrix.width() - 1)))
        ball[i][2] *= -1;
      if((ball[i][1] == 0) || (ball[i][1] == (matrix.height() - 1)))
        ball[i][3] *= -1;
      }
      delay(5);
   } else {

    delay(15);
   }
   
   // Draw big scrolly text on top
   matrix.setTextColor(matrix.Color333(7, 7, 7));
  // matrix.setTextColor(matrix.ColorHSV(hue, 255, 255, true));
   matrix.setCursor(textX, 1);
   matrix.print((message));
   //delay(15);

   // Move text left (w/wrap), increase hue
   if((--textX) < textMin) textX = matrix.width();
   hue += 1;
   if(hue >= 1536) hue -= 1536;

   // Update display
   matrix.swapBuffers(false);
   j++;
  }
  j=0;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600); 
  pinMode(12, INPUT);     // DHT - ...
  // Thermo-Hygro
  dht.begin();

  // Barometer
  if (!pressure.begin()) {
    Serial.println("Could not find a valid BMP085 sensor, check wiring!");
  }
  
  matrix.begin();
  
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
  }
  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }
}

void loop() {
  Display_Data();
  matrix.setTextWrap(false); // Allow text to run off right edge
  matrix.setTextSize(2);
  Scroll_Text();
}
