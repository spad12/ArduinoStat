#include <Adafruit_ssd1306syp.h>

#include "DHT.h"
#include "IR_AC.h"

#define SDA_PIN 9
#define SCL_PIN 8
#define DHTPIN 5

#define DHTTYPE DHT11   // DHT 22  (AM2302), AM2321

#define outputA 2
#define outputB 4

#define IRPin 3

#define MODE_AUTO 0
#define MODE_COOL 1
#define MODE_DRY 2
#define MODE_FAN 3
#define MODE_HEAT 4

doubleUint16 leader = {3287,1631};
doubleUint16 trailer = {421,8000};
doubleUint16 zeroBit = {421,421};
doubleUint16 oneBit = {421,1210};

Adafruit_ssd1306syp display(SDA_PIN,SCL_PIN);
DHT dht(DHTPIN, DHTTYPE);

IRTransmitter fj_ir(leader,trailer,zeroBit,oneBit,38);


int counter = 64; 
volatile boolean fired=false;
volatile boolean up=false;
bool is_on = false;
int aState;
int aLastState;
int bState;
// Interrupt Service Routine for a change to encoder pin A
void isr ()
{
 if (digitalRead (outputA))
   up = digitalRead (outputB);
 else
   up = !digitalRead (outputB);
 fired = true;
}  // end of isr

char getTempCode(int temp)
{
  char res = (temp-60)/2;
  return (res | (is_on)) << 4;
}

char getTimerCode(void)
{
  return 0x00;
}

char getMasterCode(int mode)
{
  return char(mode);
}

char getSwingMode(void)
{
  return 1;
}

char getFanSpeed(void)
{
  return 0;
}

unsigned char reverse(unsigned char b) {
   b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
   b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
   b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
   return b;
}

unsigned char swap_bytes(unsigned char b)
{
  b = (b << 4) | (b >> 4);
  return b;
}
void sendWordString(int temp, int mode)
{
  char chars[16];
  chars[0] = 0x14;
  chars[1] = 0x63;
  chars[2] = 0x00;
  chars[3] = 0x10;
  chars[4] = 0x10;
  chars[5] = 0xFE;
  chars[6] = 0x09;
  chars[7] = 0x30;
  chars[8] = getTempCode(temp);
  chars[9] = 0x00 | getMasterCode(mode);
  chars[10] = 0x00; 
  chars[11] = 0x00;
  chars[12] = 0x00;
  chars[13] = 0x00;
  chars[14] = 0x20;
  chars[15] = 0;

  for(int i=7;i<15;i++)
  {
    chars[15] += (chars[i]);

  }

  chars[15] = (0x100-chars[15]);
  fj_ir.transmitWords(chars,16);
}
void setup()
{
  pinMode(IRPin,OUTPUT);
  digitalWrite (outputA,HIGH);
  digitalWrite (outputB,HIGH);
  digitalWrite (IRPin,LOW);
  delay(100);
  display.initialize();
  aLastState = digitalRead(outputA);  
  display.setTextSize(1);
  display.setTextColor(WHITE);

  Serial.begin(9600);
 attachInterrupt (0, isr, CHANGE);

}
void loop()
{
  int counter2 = counter;
  if(fired)
  {
    if(up)
    counter+=2;
    else
    counter-=2;

  counter = min(counter,88);
  counter = max(counter,64);
  
  sendWordString(counter,MODE_COOL);
  }
  fired=false;



  display.clear();
  display.setCursor(0,0);
  display.print("Temp Set: ");
  display.println(counter);
  display.print("Current Temp: ");
  display.println((dht.readTemperature(true)),0);
  display.print("Current Humid:  ");
  display.print((dht.readHumidity()),0);
  display.println();
  display.update();
  delay(200);
}
