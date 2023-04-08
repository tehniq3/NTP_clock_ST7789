/*
 An example analogue clock using a TFT LCD screen to show the time
 use of some of the drawing commands with the library.
 Based on a sketch by Gilchrist 6/2/2014 1.0

 Nicu FLORICA (niq_ro) made this changes:
 v.0 - added digital clock on this analog pseudoclock
 v.1 - clean the sketch + added NTP clock + DST switch (summer/winter time)
 v.2 - changed to 240x240 (ST7789 - 1.3") + DST selection is with analog pin (GND or 3.3V) 
      * for ST7789 driver it need to change also settings in User_setup.h file inside of TFT_eSPI library
 v.2a - changed to have just big analog clock for a minute and small analog clock + digital cock other minute
 */

#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <SPI.h>
#include <TFT_eSPI.h> // https://github.com/Bodmer/TFT_eSPI

#define TFT_GREY 0x5AEB

TFT_eSPI tft = TFT_eSPI();       // Invoke custom library

float sx = 0, sy = 1, mx = 1, my = 0, hx = -1, hy = 0;    // Saved H, M, S x & y multipliers
float sdeg=0, mdeg=0, hdeg=0;
uint16_t osx=120, osy=120, omx=120, omy=120, ohx=120, ohy=120;  // Saved H, M, S x & y coords
uint16_t x0=0, x1=0, yy0=0, yy1=0;
uint32_t targetTime = 0;                    // for next 1 second timeout

bool initial = 1;

byte omm = 99, oss = 99;
byte omm2 = 99;  // new value for old minute
byte xcolon = 0, xsecs = 0;
unsigned int colour = 0;

const char *ssid     = "niq_ro";
const char *password = "berelaburtica";
const long timezoneOffset = 2*3600; // GMT + seconds  (Romania)

//#define DSTpin  12 // GPIO12 = D6, see https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/
#define DSTpin  A0 //  see https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/


// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", timezoneOffset);

int hh, mm, ss;
byte DST = 0;
byte DST0;
unsigned long tpcitire;

void setup(void) {
  pinMode (DSTpin, INPUT);
  Serial.begin(115200);
  Serial.println (" ");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay (500);
    Serial.print (".");
  }
    Serial.println (" ");
    
//  if (digitalRead(DSTpin) == LOW)
  if (analogRead(DSTpin) < 500)
   DST = 0;
  else
   DST = 1;
  timeClient.setTimeOffset(timezoneOffset + DST*3600);
  timeClient.begin();
  DST0 = DST;
  
  tft.init();
  tft.setRotation(0);
  
  //tft.fillScreen(TFT_BLACK);
  //tft.fillScreen(TFT_RED);
  //tft.fillScreen(TFT_GREEN);
  //tft.fillScreen(TFT_BLUE);
  //tft.fillScreen(TFT_BLACK);
  tft.fillScreen(TFT_GREY);
  
  tft.setTextColor(TFT_WHITE, TFT_BLACK);  // Adding a background colour erases previous text automatically
  tft.fillCircle(120, 91, 3, TFT_WHITE);

  // Draw text at position 120,260 using fonts 4
  // Only font numbers 2,4,6,7 are valid. Font 6 only contains characters [space] 0 1 2 3 4 5 6 7 8 9 : . - a p m
  // Font 7 is a 7 segment font and only contains characters [space] 0 1 2 3 4 5 6 7 8 9 : .
 // tft.drawCentreString("Time flies",120,260,4);
}

void loop() {
// if ((digitalRead(DSTpin) == LOW) && (initial == 0))
   if (analogRead(DSTpin) < 500)
   DST = 0;
  else
   DST = 1;

if (DST != DST0)
{
  timeClient.setTimeOffset(timezoneOffset + DST*3600);
  timeClient.begin();
  timeClient.update();
 hh = timeClient.getHours();
 mm = timeClient.getMinutes();
 ss = timeClient.getSeconds();
 tpcitire = millis();
//DST0 = DST;
initial = 1;
//omm = 70;
//oss = 70;
}

if (millis() - tpcitire > 500)
{
timeClient.update();
 hh = timeClient.getHours();
 mm = timeClient.getMinutes();
 ss = timeClient.getSeconds();
 tpcitire = millis();
 Serial.print(hh);
 Serial.print(":");
 Serial.println(mm);
}
/*
 ora = ora%12;  // 12-hour format
 if (ora == 0) ora = 12;  // 12-hour format
*/

    // Pre-compute hand degrees, x & y coords for a fast screen update
    sdeg = ss*6;                  // 0-59 -> 0-354
    mdeg = mm*6+sdeg*0.01666667;  // 0-59 -> 0-360 - includes seconds
    hdeg = hh*30+mdeg*0.0833333;  // 0-11 -> 0-360 - includes minutes and seconds
    hx = cos((hdeg-90)*0.0174532925);    
    hy = sin((hdeg-90)*0.0174532925);
    mx = cos((mdeg-90)*0.0174532925);    
    my = sin((mdeg-90)*0.0174532925);
    sx = cos((sdeg-90)*0.0174532925);    
    sy = sin((sdeg-90)*0.0174532925);

if (initial == 1)
{
if (mm%2 == 0)
{
    // Draw clock face
  tft.fillCircle(120, 90, 88, TFT_GREEN);
  tft.fillCircle(120, 90, 80, TFT_BLACK);

  // Draw 12 lines
  for(int i = 0; i<360; i+= 30) {
    sx = cos((i-90)*0.0174532925);
    sy = sin((i-90)*0.0174532925);
    x0 = sx*84+120;
    yy0 = sy*84+90;
    x1 = sx*70+120;
    yy1 = sy*70+90;

    tft.drawLine(x0, yy0, x1, yy1, TFT_GREEN);
  }

  // Draw 60 dots
  for(int i = 0; i<360; i+= 6) {
    sx = cos((i-90)*0.0174532925);
    sy = sin((i-90)*0.0174532925);
    x0 = sx*72+120;
    yy0 = sy*72+90;
    // Draw minute markers
    tft.drawPixel(x0, yy0, TFT_WHITE);
    
    // Draw main quadrant dots
    if(i==0 || i==180) tft.fillCircle(x0, yy0, 2, TFT_WHITE);
    if(i==90 || i==270) tft.fillCircle(x0, yy0, 2, TFT_WHITE);
  }

  tft.fillCircle(120, 91, 3, TFT_WHITE);
}
  else
  {
      // Draw clock face
  tft.fillCircle(120, 120, 118, TFT_GREEN);
  tft.fillCircle(120, 120, 110, TFT_BLACK);

  // Draw 12 lines
  for(int i = 0; i<360; i+= 30) {
    sx = cos((i-90)*0.0174532925);
    sy = sin((i-90)*0.0174532925);
    x0 = sx*114+120;
    yy0 = sy*114+120;
    x1 = sx*100+120;
    yy1 = sy*100+120;

    tft.drawLine(x0, yy0, x1, yy1, TFT_GREEN);
  }

  // Draw 60 dots
  for(int i = 0; i<360; i+= 6) {
    sx = cos((i-90)*0.0174532925);
    sy = sin((i-90)*0.0174532925);
    x0 = sx*102+120;
    yy0 = sy*102+120;
    // Draw minute markers
    tft.drawPixel(x0, yy0, TFT_WHITE);
    
    // Draw main quadrant dots
    if(i==0 || i==180) tft.fillCircle(x0, yy0, 2, TFT_WHITE);
    if(i==90 || i==270) tft.fillCircle(x0, yy0, 2, TFT_WHITE);
  }

  tft.fillCircle(120, 121, 3, TFT_WHITE);
  }
}

if (mm%2 == 0) 
{
  //  if (ss==0 || initial) {
     if ((ss==0) || (DST0 != DST) || initial) {
      initial = 0;
      // Erase hour and minute hand positions every minute
      tft.drawLine(ohx, ohy, 120, 91, TFT_BLACK);
      ohx = hx*60+121;    
      ohy = hy*60+91;
      tft.drawLine(omx, omy, 120, 91, TFT_BLACK);
      omx = mx*74+120;    
      omy = my*74+91;
    }

      // Redraw new hand positions, hour and minute hands not erased here to avoid flicker
      tft.drawLine(osx, osy, 120, 91, TFT_BLACK);
      osx = sx*80+121;    
      osy = sy*80+91;
      tft.drawLine(osx, osy, 120, 91, TFT_RED);
      tft.drawLine(ohx, ohy, 120, 91, TFT_WHITE);
      tft.drawLine(omx, omy, 120, 91, TFT_WHITE);
      tft.drawLine(osx, osy, 120, 91, TFT_RED);

    tft.fillCircle(120, 91, 3, TFT_RED);

   // Update digital time
    int xpos = 5;
    int ypos = 190;
    int ysecs = ypos;

  //   if (omm != mm) { // Redraw hours and minutes time every minute
      if ((omm != mm) || (DST0 != DST)) {
      omm = mm;
      DST0 = DST;
      // Draw hours and minutes
      if (hh < 10) xpos += tft.drawChar('0', xpos, ypos, 7); // Add hours leading zero for 24 hr clock
      xpos += tft.drawNumber(hh, xpos, ypos, 7);             // Draw hours
      xcolon = xpos; // Save colon coord for later to flash on/off later
      xpos += tft.drawChar(':', xpos, ypos - 8, 6);
      if (mm < 10) xpos += tft.drawChar('0', xpos, ypos, 7); // Add minutes leading zero
      xpos += tft.drawNumber(mm, xpos, ypos, 7);             // Draw minutes
      xsecs = xpos; // Sae seconds 'x' position for later display updates
    } 
      
   if (oss != ss) { // Redraw seconds time every second
      oss = ss;
      xpos = xsecs;   
      xpos += tft.drawChar(':', xsecs, ysecs, 6); // Seconds colon\-,gg
      if (ss % 2) { // Flash the colons on/off
      tft.drawChar(' ', xcolon, ypos, 6);     // Hour:minute colon
      tft.drawChar(' ', xsecs, ysecs, 6);     // Hour:minute colon
     }
      else {
        tft.drawChar(':', xcolon, ypos, 6);     // Hour:minute colon
      }
       
      //Draw seconds
      if (ss < 10) xpos += tft.drawChar('0', xpos, ysecs, 7); // Add leading zero
      tft.drawNumber(ss, xpos, ysecs, 7);                     // Draw seconds
    }
}
else  // minute is odd
{
 //   if (ss==0 || initial) {
     if ((ss==0) || (DST0 != DST) || initial) {
      initial = 0;
      // Erase hour and minute hand positions every minute
      tft.drawLine(ohx, ohy, 120, 121, TFT_BLACK);
      ohx = hx*62+121;    
      ohy = hy*62+121;
      tft.drawLine(omx, omy, 120, 121, TFT_BLACK);
      omx = mx*84+120;    
      omy = my*84+121;
    }

      // Redraw new hand positions, hour and minute hands not erased here to avoid flicker
      tft.drawLine(osx, osy, 120, 121, TFT_BLACK);
      osx = sx*90+121;    
      osy = sy*90+121;
      tft.drawLine(osx, osy, 120, 121, TFT_RED);
      tft.drawLine(ohx, ohy, 120, 121, TFT_WHITE);
      tft.drawLine(omx, omy, 120, 121, TFT_WHITE);
      tft.drawLine(osx, osy, 120, 121, TFT_RED);

    tft.fillCircle(120, 121, 3, TFT_RED);

   // Update digital time
    int xpos = 10;
    int ypos = 260;
    int ysecs = ypos;

 //    if (omm != mm) { // Redraw hours and minutes time every minute or DST mode was changed
      if ((omm != mm) || (DST0 != DST)) {
      omm = mm;
      DST0 = DST;
      // Draw hours and minutes
      if (hh < 10) xpos += tft.drawChar('0', xpos, ypos, 7); // Add hours leading zero for 24 hr clock
      xpos += tft.drawNumber(hh, xpos, ypos, 7);             // Draw hours
      xcolon = xpos; // Save colon coord for later to flash on/off later
      xpos += tft.drawChar(':', xpos, ypos - 8, 6);
      if (mm < 10) xpos += tft.drawChar('0', xpos, ypos, 7); // Add minutes leading zero
      xpos += tft.drawNumber(mm, xpos, ypos, 7);             // Draw minutes
      xsecs = xpos; // Sae seconds 'x' position for later display updates
    } 
      
   if (oss != ss) { // Redraw seconds time every second
      oss = ss;
      xpos = xsecs;
     // tft.setTextColor(TFT_WHITE, TFT_BLACK);  // Adding a background colour erases previous text automatically  
      xpos += tft.drawChar(':', xsecs, ysecs, 6); // Seconds colon\-,gg
      if (ss % 2) { // Flash the colons on/off
      tft.drawChar(' ', xcolon, ypos, 6);     // Hour:minute colon
      tft.drawChar(' ', xsecs, ysecs, 6);     // Hour:minute colon
     }
      else {
        tft.drawChar(':', xcolon, ypos, 6);     // Hour:minute colon
      }
       
      //Draw seconds
      if (ss < 10) xpos += tft.drawChar('0', xpos, ysecs, 7); // Add leading zero
      tft.drawNumber(ss, xpos, ysecs, 7);                     // Draw seconds
    }  
}

if(omm2 != mm)
{
  initial = 1;
  tft.fillScreen(TFT_BLACK);  
  tft.setTextColor(TFT_WHITE, TFT_BLACK);  // Adding a background colour erases previous text automatically 
  oss = 90; 
  omm = 90;
  DST0 = 99;
}
omm2 = mm;

delay(50);
}  // main loop

static uint8_t conv2d(const char* p) {
  uint8_t v = 0;
  if ('0' <= *p && *p <= '9')
    v = *p - '0';
  return 10 * v + *++p - '0';
}
