

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>

#include <ESP8266WiFi.h>
#include <NTPClient.h> 
#include <WiFiUdp.h> 
#include "OneButton.h"

#include "icons.h"
#include "Ticker.h"


// ST7789 TFT module connections
#define TFT_DC    D4    // TFT DC  pin is connected to NodeMCU pin D1 (GPIO5)
#define TFT_RST   D8     // TFT RST pin is connected to NodeMCU pin D2 (GPIO4)
#define TFT_CS    -1     // TFT CS  pin is connected to NodeMCU pin D8 (GPIO15)
// initialize ST7789 TFT library with hardware SPI module
// SCK (CLK) ---> NodeMCU pin D5 (GPIO14)
// MOSI(DIN) ---> NodeMCU pin D7 (GPIO13)
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);


/*#include <SPI.h>
#define BME_SCK 14
#define BME_MISO 12
#define BME_MOSI 13
#define BME_CS 15*/

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme; // I2C
//Adafruit_BME280 bme(BME_CS); // hardware SPI
//Adafruit_BME280 bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK); // software SPI

unsigned long delayTime;

String inHum;
String inTemp;

String SSID = "Acraft";
String PASS = "12345678";

const long utcOffsetInSeconds = 7200+3600;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

int buttonPin = D6;
int signalPin = D0;
int signalState = LOW;
OneButton button(buttonPin, true);

Ticker alarm;

int Sx[60],Mx[60],Hx[30];
int Sy[60],My[60],Hy[30];
int center_x=120;
int center_y=120;
int seconds_hand_len = 80;
int minutes_hand_len = 60;
int hours_hand_len = 40;
int clock_radius=90;
int hours_x=0;
int hours_y=0;

int hours;
int minutes;
int seconds;

int prevS;
int prevM;
int prevH;

void printValues();
void singleClick();
void changeState();

void drawClock();

void displayAnalogueClock();

long int currentMillis = 0;
long int startMillis = 0;

int period = 1000;





void setup() {
  Serial.begin(115200);
  Serial.println(F("BME280 test"));

  bool status;

  pinMode(buttonPin,INPUT_PULLUP);
  pinMode(signalPin,OUTPUT);
  digitalWrite(signalPin,signalState);

  // default settings
  // (you can also pass in a Wire library object like &Wire2)
  status = bme.begin(0x76);  
  if (!status) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }

  Serial.println("-- Default Test --");
  delayTime = 1000;

  Serial.println();

  tft.init(240, 240, SPI_MODE3);    // Init ST7789 display 240x240 pixel

  // if the screen is flipped, remove this command
  //ft.setRotation(2);

  Serial.println(F("Initialized"));

  
  tft.fillScreen(ST77XX_BLACK);
  
  //tft.drawRGBBitmap(160,160,a02d_64,64,64);

  Serial.print("Connecting to WiFi");
  WiFi.begin(SSID, PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }

  timeClient.begin();
  if (timeClient.update()) {
      Serial.println("Synchronized with NTP.");
    } else {
      Serial.println("Error synchronizing with NTP.");
    }
  

hours = timeClient.getHours();
minutes = timeClient.getMinutes();
seconds = timeClient.getSeconds();

Serial.print(hours);Serial.print(":");
Serial.print(minutes);Serial.print(":");
Serial.print(seconds);Serial.println("");
  
  pinMode(buttonPin,INPUT_PULLUP);
  pinMode(signalPin,OUTPUT);
  digitalWrite(signalPin,signalState);

  button.attachClick(singleClick); 

 drawClock();
 
  prevS=seconds;
 prevM=minutes;
 prevH=hours;
}

void loop() { 
  button.tick();
 
 currentMillis = millis();
  

if(currentMillis - startMillis >= period) {
  
  displayAnalogueClock();
  startMillis=currentMillis; 
}

}

void displayAnalogueClock(){
   hours = timeClient.getHours();
   minutes = timeClient.getMinutes();
   seconds = timeClient.getSeconds();
  
  /*
  Serial.print(hours);Serial.print(":");
  Serial.print(minutes);Serial.print(":");
  Serial.print(seconds);Serial.println("");
  */  
       tft.drawLine(120,120,Sx[prevS],Sy[prevS],ST77XX_BLACK); 
       tft.drawLine(120,120,Sx[seconds],Sy[seconds],ST77XX_GREEN); 
       prevS=seconds;
    
       tft.drawLine(120,120,Mx[prevM],My[prevM],ST77XX_BLACK);
       tft.drawLine(120,120,Mx[minutes],My[minutes],ST77XX_YELLOW); 

       hours_x = int(hours_hand_len * sin(radians(prevH * 30 + 0.5 * prevM)) + center_x);
       hours_y = int(-1 * hours_hand_len * cos(radians(prevH * 30 + 0.5 * prevM )) + center_y);
       prevM=minutes;

       tft.drawLine(120,120,hours_x,hours_y,ST77XX_BLACK);

       
       hours_x = int(hours_hand_len * sin(radians(hours * 30 + 0.5 * minutes)) + center_x);
       hours_y = int(-1 * hours_hand_len * cos(radians(hours * 30 + 0.5 * minutes )) + center_y);
       tft.drawLine(120,120,hours_x,hours_y,ST77XX_RED);
       prevH=hours;
      

}

void printValues() {
  Serial.print("Temperature = ");
  Serial.print(bme.readTemperature());
  Serial.println(" *C");
  
  // Convert temperature to Fahrenheit
  /*Serial.print("Temperature = ");
  Serial.print(1.8 * bme.readTemperature() + 32);
  Serial.println(" *F");*/
  
  Serial.print("Pressure = ");
  Serial.print(bme.readPressure() / 100.0F);
  Serial.println(" hPa");

  Serial.print("Approx. Altitude = ");
  Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
  Serial.println(" m");

  Serial.print("Humidity = ");
  Serial.print(bme.readHumidity());
  Serial.println(" %");

  Serial.println();

 inHum=  String(bme.readHumidity(),1)+"%";
  inTemp= String(bme.readTemperature(),1)+"C";
  tft.setTextWrap(false);
  //tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(0, 30);
  tft.setTextColor(ST77XX_RED,0);
  tft.setTextSize(6);
  tft.println(inTemp);
  tft.setTextColor(ST77XX_YELLOW,0);
  tft.println(inHum);
 
  
  //tftPrintTest() ;
  //drawClock();
  

}




void changeState()
{
  digitalWrite(signalPin,!digitalRead(signalPin));
}

void singleClick()
{ 
  Serial.println("SingleClick() detected.");
  printValues();
  if(signalState == LOW) {
    alarm.detach();
    digitalWrite(signalPin, LOW);}
  else {
    alarm.attach(0.5,changeState);}

  signalState = !signalState;
  
}

void drawClock() {

for(int secs=0;secs<60;secs++){
 Sx[secs] = int(seconds_hand_len * sin(radians(secs * 6)) + center_x);
 Sy[secs] = int(-1 * seconds_hand_len * cos(radians(secs * 6)) + center_y);
}

for(int mins=0;mins<60;mins++){
 Mx[mins] = int(minutes_hand_len * sin(radians(mins * 6)) + center_x);
 My[mins] = int(-1 * minutes_hand_len * cos(radians(mins * 6)) + center_y);
}
 // tft.begin();

  tft.drawCircle(center_x,center_y,clock_radius,ST77XX_WHITE);
  tft.drawCircle(center_x,center_y,clock_radius+1,ST77XX_WHITE);
 
for(int hrs=0;hrs<12;hrs++){

hours_x = int(clock_radius * sin(radians(hrs * 30 )) + center_x);
hours_y = int(-1 *clock_radius * cos(radians(hrs * 30)) + center_y);
tft.drawLine(120,120,hours_x,hours_y,ST77XX_WHITE);
}
  tft.fillCircle(120,120,clock_radius-10,ST77XX_BLACK);

tft.setTextSize(2);

  tft.setCursor(center_x-4,center_y-clock_radius-16);
  tft.print("12");
  tft.setCursor(center_x-4,center_y+clock_radius+10);
  tft.print("6");
  tft.setCursor(center_x+clock_radius+10,center_y-4);
  tft.print("3");
  tft.setCursor(center_x-clock_radius-12,center_y-4);
  tft.print("9");
 

hours_x = int(hours_hand_len * sin(radians(hours * 30 + 0.5 * minutes)) + center_x);
hours_y = int(-1 * hours_hand_len * cos(radians(hours * 30 + 0.5 * minutes )) + center_y);
       

 tft.drawLine(120,120,Sx[seconds],Sy[seconds],ST77XX_GREEN);
 tft.drawLine(120,120,Mx[minutes],My[minutes],ST77XX_YELLOW);
 tft.drawLine(120,120,hours_x,hours_y,ST77XX_RED);

}


