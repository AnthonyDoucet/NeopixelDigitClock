#include <FastLED.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include "RTClib.h"

//Bluetooth
SoftwareSerial BTSerial =  SoftwareSerial(4, 5);

//Horloge
RTC_DS3231 rtc;

//Neopixel
#define STRIP_PIN 6
#define NUMPIXELS 86          //21*4+2 = 86
#define PIXEL_PER_SEGMENT  3
#define PIXEL_SEGMENT_NUMBER 7

CRGB leds[NUMPIXELS] = {0};

//Variables Globales
int lastHour, lastMin, lastSec;
byte hue, color_R, color_G, color_B;
bool etatSeparateur;

//Masque Digit
const bool pixelValues[11][21] = {
  {1,1,1, 1,1,1, 1,1,1, 1,1,1, 1,1,1, 1,1,1, 0,0,0}, // Number 0
  {0,0,0, 0,0,0, 1,1,1, 1,1,1, 0,0,0, 0,0,0, 0,0,0}, // Number 1
  {0,0,0, 1,1,1, 1,1,1, 0,0,0, 1,1,1, 1,1,1, 1,1,1}, // Number 2
  {0,0,0, 1,1,1, 1,1,1, 1,1,1, 1,1,1, 0,0,0, 1,1,1}, // Number 3
  {1,1,1, 0,0,0, 1,1,1, 1,1,1, 0,0,0, 0,0,0, 1,1,1}, // Number 4
  {1,1,1, 1,1,1, 0,0,0, 1,1,1, 1,1,1, 0,0,0, 1,1,1}, // Number 5
  {1,1,1, 1,1,1, 0,0,0, 1,1,1, 1,1,1, 1,1,1, 1,1,1}, // Number 6
  {0,0,0, 1,1,1, 1,1,1, 1,1,1, 0,0,0, 0,0,0, 0,0,0}, // Number 7
  {1,1,1, 1,1,1, 1,1,1, 1,1,1, 1,1,1, 1,1,1, 1,1,1}, // Number 8
  {1,1,1, 1,1,1, 1,1,1, 1,1,1, 1,1,1, 0,0,0, 1,1,1}, // Number 9
  {0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0}  // Off
};


void setup() {
  //Init Serial
  Serial.begin(115200); //USB
  BTSerial.begin(9600); //Bluetooth
  
  Serial.println("Horloge Start");
  Serial.println("-------------------");

  //Init Neopixel
  pinMode(STRIP_PIN, OUTPUT);
  FastLED.addLeds<WS2812B, STRIP_PIN, GRB>(leds, NUMPIXELS);
  FastLED.setBrightness(255);

  //Init Horloge RTC
  while(!rtc.begin()){
    Serial.println("Couldn't find RTC");
    for(int i=0 ; i<NUMPIXELS ; i++){
      leds[i] = CRGB(255,0,0);
      delay(50);
      leds[i-3] = CRGB(0,0,0);
      FastLED.show();
    }
    FastLED.clear();
  }
  
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, lets set the time!");
    fill_solid(leds, NUMPIXELS, CRGB(0,255,0));
    FastLED.show();
    delay(1000);
    FastLED.clear();
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  //First Time Update
  updateTime();
}


void loop() {

  serialEvent();
  updateTime();

  FastLED.show();
}


void serialEvent(){
    
    if(Serial.available() > 0){
      
      String str = Serial.readString();

      if(str[0] == 'R'){
        Serial.print("Rouge ");
        Serial.println(color_R);
      }
      else if(str[0] == 'G'){
        Serial.print("Vert ");
        Serial.println(color_G);
      }
      else if(str[0] == 'B'){
        Serial.print("Bleu ");
        Serial.println(color_B);
      }
      
      /*
      if(str != ""){
        Serial.print(str);
        Serial.println();
        String buff;
        
        buff = str.substring(1, str.indexOf(';'));
        color_R = buff.toInt();
        Serial.print("R: ");
        Serial.println(color_R);
        
        
        buff = str.substring(1, str.indexOf(';')+1);
        color_G = buff.toInt();
        Serial.print("G: ");
        Serial.println(color_G);
      }
      */
    }
    
}


void updateTime(){
  DateTime now = rtc.now();

  /*
  Serial.print("Time = ");
  Serial.print(now.hour());
  Serial.print("h");
  Serial.print(now.minute());
  Serial.print("m");
  Serial.print(now.second());
  Serial.println("s");
  */

  if(now.hour() != lastHour){
    lastHour = now.hour();
    
    Serial.println("Update Heures");
    byte num1 = (now.hour() /10) % 10;
    byte num2 = now.hour() % 10;
    writeDigit(1, num1);
    writeDigit(2, num2);
  }
  

  if(now.minute() != lastMin){
    lastMin = now.minute();
    
    Serial.println("Update Minutes");
    byte num3 = (now.minute() /10) % 10;
    byte num4 = now.minute() % 10;
    writeDigit(3, num3);
    writeDigit(4, num4);
  }

  
  if(now.second() != lastSec){
    lastSec = now.second();
    etatSeparateur = !etatSeparateur;
    //hue += 10;
    writeDigit(5,etatSeparateur);
  }

}


void writeDigit(int index, int val) {

  int currentPixel = 0;
  int compteur = 0;
  bool separateur = false;
  
  switch(index){
      case 1:
              currentPixel = 0;
              break;
      
      case 2:
              currentPixel = 21;
              break;
      
      case 3:
              currentPixel = 44;
              break;
      
      case 4:
              currentPixel = 65;
              break;
      
      case 5: //Separateur
              currentPixel = 42;
              if(val == 1){
                leds[currentPixel] = CRGB::Green;
                leds[currentPixel+1] = CRGB::Green;
              }
              if(val == 0){
                leds[currentPixel] = CRGB(0,0,0);
                leds[currentPixel+1] = CRGB(0,0,0);
              }
              separateur = true;
              break;
  }
  /*
  Serial.print("Index: ");
  Serial.print(index);
  Serial.print(" Val: ");
  Serial.print(val);
  Serial.println("  ");
  */

  if(!separateur){
    for(int i=0 ; i < PIXEL_SEGMENT_NUMBER ; i++){
      for(int j=0 ; j < PIXEL_PER_SEGMENT ; j++){
        
        if(pixelValues[val][compteur] == 1){
          leds[currentPixel+compteur] = CRGB(0,255,0);
        }
        else{
          leds[currentPixel+compteur] = CRGB(0,0,0);
        }
        compteur += 1;
        
      }
    }
  }
  
}
