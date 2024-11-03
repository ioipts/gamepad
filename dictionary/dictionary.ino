/**
 * LOLIN(WEMOS) D1 R2 & Mini
 * Green SPI_MODE2
 * Blue SPI_MODE3
 */

//#define _PCF8574AP
//#define _MODE3
const char* id="MEKMOK_DIC_GAME"; 
const char* ver="20240603";

#include "user_interface.h"
#include <ESP8266WiFi.h>  
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <SoftwareSerial.h>
#include <WiFiClient.h>
#include <Wire.h>
#include "PCF8574.h"
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789

#define ARRAYSIZE 12
String text[ARRAYSIZE]={"I\nHAVE\nA\nBALL","I\nLIKE\nTO\nPLAY","A\nCUP\nOF\nWATER","TAKE\nTHE\nBUS","WASH\nYOUR\nHAND","HEAD\nKNEE\nTOE","BARE\nFOOT","TURN\nON\nTHE\nFAN","MOMMY\nAND\nDADDY","STAY\nHOME","FLOOR\nWALL\nCEILING","SHIRT\nAND\nPANT"};
#define COLORSIZE 5
int color[COLORSIZE]={ST77XX_BLACK,ST77XX_BLUE,ST77XX_RED,ST77XX_MAGENTA,ST77XX_GREEN};

WiFiClient wifiClient;
ESP8266WebServer server(80); 
bool pcf8574;

#if defined(_PCF8574AP)
PCF8574 pcf(0x38);
#define PCFADDR 0x38
#else
PCF8574 pcf(0x20);
#define PCFADDR 0x20
#endif

Adafruit_ST7789 tft=Adafruit_ST7789(10,D8,D6);  //DC,RS
unsigned long myTime;
String textword="";
int i=0;
int j=0;
bool flip=true;

void handleRoot() {
  char msg[2048];  
  sprintf(msg,"<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><style> a:link, a:visited {color: white;padding: 5px 5px;text-align: center;text-decoration: none;display: inline-block;} a:hover, a:active { background-color: #0070a1; }</style></head><body style=\"background-color:#6491c2\"><br/><br/><div align=\"center\"><h1><b>New Text</b></h1></div><br/><br/><div align=\"center\" style=\"background-color:white;position: relative;margin: auto;width:400px;border:1px solid;\"><br/><form action=\"rpeng\" method=\"post\"><table><tr><td><input type=\"text\" name=\"text\" value=\"\" length=90></td></tr><tr><td colspan=\"2\" align=\"center\"><input type=\"submit\" style=\"width: 100px;\" value=\"&nbsp;OK&nbsp;\"></td></tr></table><br/></form></div><br/><div align=\"center\">version %s</div></body></html>",ver);
  server.send(200, "text/html", msg);   
}

void handleSubmit() {
  char msg[2048];
  if(!server.hasArg("text")) { 
    server.send(400, "text/plain", "400: Invalid Request");        
    return;
  }
  textword=server.arg("text");
  sprintf(msg,"<!DOCTYPE html><html><head><meta charset=\"UTF-8\"></head><body style=\"background-color:#6491c2\"><br/><br/><div align=\"center\"><h1><b>%s</b></h1></div><br/></body></html>",textword); 
  flip=true;
  server.send(200, "text/html", msg);
}


void setup() {
  pinMode(D0, OUTPUT);
  digitalWrite(D0, HIGH);         //enable display
  for (int i=0;i<=5;i++)
  {
   pcf.pinMode(i,INPUT);      
   pcf.pinMode(i, INPUT_PULLUP);
  }
  pcf.begin();
  
  Wire.begin( D2,D1);             //SDA, SCL
  Wire.setClock( 100000 );        //26/1/2020 tested with oled work for some sensor if remove
  Wire.beginTransmission(PCFADDR);
  delay(10);
  pcf8574=(!Wire.endTransmission());

  struct station_config conf;
  wifi_station_get_config(&conf);
  wifi_status_led_uninstall();
  WiFi.mode(WIFI_AP_STA);      
  WiFi.hostname(id);        
  WiFi.softAP(id, "");    
  server.on("/", handleRoot);    
  server.begin();     

  tft.init(240,240);
#if defined(_MODE3)
  tft.init(240, 240, SPI_MODE3); 
#else
  tft.init(240, 240, SPI_MODE2); 
#endif
  // tft.fillScreen(ST77XX_WHITE);  
  tft.setRotation(2); 
  myTime=millis();
}

void draw(int c,String text) {
     tft.fillScreen(c);
     tft.setCursor(0,10);
     tft.setTextColor(ST77XX_WHITE , c);
     tft.setTextSize(7);
     tft.print(text);
     //tft.drawRGBBitmap(20,20,board,100,100);
}

void loop () {
  unsigned long now = millis();
  unsigned long d=now-myTime;
  if (pcf8574) {
    if (pcf.digitalRead(0)==LOW) {
      i=i-1;
      if (i<0) i=0;
      else flip=true;
      textword="";
    } 
    if (pcf.digitalRead(3)==LOW) {
      i=i+1;
      if (i>=ARRAYSIZE) i=ARRAYSIZE-1;
      else flip=true;
      textword="";
    }  
    if (pcf.digitalRead(1)==LOW) {
       j=j-1;
      if (j<0) j=0;
      else flip=true;
    } 
    if (pcf.digitalRead(2)==LOW) {
      j=j+1;
      if (j>=COLORSIZE) j=COLORSIZE-1;
      else flip=true;      
    } 
  }
  if (flip) {
    if (textword!="") draw(color[j],textword);
    else draw(color[j],text[i]);
    flip=false;
  }
  myTime=now;
  server.handleClient();  
}
