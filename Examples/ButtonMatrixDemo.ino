/* Illuminated Button Matrix Demo

   David Johnson-Davies - www.technoblogy.com - 9th May 2019
   Arduino Uno
   
   CC BY 4.0
   Licensed under a Creative Commons Attribution 4.0 International license: 
   http://creativecommons.org/licenses/by/4.0/
*/
#include <Wire.h>

#define Game ToyakiPlus
// #define Game Toyaki

const int I2CAddress = 0x3A;
const int Int0 = 2;
unsigned int Lights;

void Display (unsigned int lights) {
  Wire.beginTransmission(I2CAddress);
  Wire.write(lights & 0xFF);
  Wire.write(lights>>8 & 0xFF);
  Wire.endTransmission(); 
}

unsigned int ReadKeys () {
  uint8_t Low, High;
  Wire.requestFrom(I2CAddress, 2);
  Low = Wire.read();
  High = Wire.read();
  return High<<8 | Low;
}

void ToyakiPlus (unsigned int Keys) {
  unsigned int Previous = Keys>>4;
  unsigned int Next = Keys<<4;
  unsigned int Right = Keys<<1 & 0xEEEE;
  unsigned int Left = Keys>>1 & 0x7777;
  Lights = Lights ^ (Previous | Next | Right | Left | Keys);
}

void Toyaki (unsigned int Keys) {
  unsigned int All = Keys;
  unsigned int NorthEast = Keys, NorthWest = Keys, SouthEast = Keys, SouthWest = Keys;
  for (int i=0; i<3; i++) {
    NorthEast = (NorthEast & 0x7777)>>3; All = All | NorthEast;
    NorthWest = (NorthWest & 0xEEEE)>>5; All = All | NorthWest;
    SouthEast = (SouthEast & 0x7777)<<5; All = All | SouthEast;
    SouthWest = (SouthWest & 0xEEEE)<<3; All = All | SouthWest;
  }
  Lights = Lights ^ All;
}

void setup() {
  Wire.begin();
}
  
void loop() {
  // Generate random starting position
  Lights = 0;
  for (int i=0; i<16; i++) Game(1<<(random(16)));
  Display(Lights);
  //
  // New game
  do {
    unsigned int Keys;
    // Wait for press
    do Keys = ReadKeys(); while (Keys == 0);
    // Read buttons
    Game(Keys);
    Display(Lights);
    delay(200);
    // Wait for release
    do Keys = ReadKeys(); while (Keys != 0);
  } while (Lights != 0);
  //
  // Solved!
  for (int i=0; i<6; i++) {
    delay(500);
    Lights = Lights ^ 0xFFFF;
    Display(Lights);
  }
}
