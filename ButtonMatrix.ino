/* Illuminated Button Matrix

   David Johnson-Davies - www.technoblogy.com - 9th May 2019
   ATtiny88 @ 8MHz (internal oscillator; BOD disabled)
   
   CC BY 4.0
   Licensed under a Creative Commons Attribution 4.0 International license: 
   http://creativecommons.org/licenses/by/4.0/
*/

#include <Wire.h>

volatile int Lights;           // One bit per light; bit 0 = Button 1 etc.
volatile int Keys;             // One bit per key; bit 0 = Button 1 etc.
int Row;

// INT pin **********************************************

const int IntPin = PORTC7;

inline void IntLow () {
  PORTC = PORTC & ~(1<<IntPin);
  DDRC = DDRC | 1<<IntPin;
}

inline void IntRelease () {
  DDRC = DDRC & ~(1<<IntPin);
  PORTC = PORTC | 1<<IntPin;
}

// I2C Interface **********************************************

const int I2CAddress = 0x3A;

void SetLights (int nBytes) {
  (void) nBytes;
  uint8_t Low = Wire.read();
  uint8_t High = Wire.read();
  Lights = High<<8 | Low;
}

void GetKeys () {
  int SendData = ReadKeys();
  Wire.write(SendData & 0xFF);
  Wire.write(SendData>>8 & 0xFF);
  IntRelease();
}

// Keys **********************************************

void SetupKeys () {
  // Set up pin change interrupts on the keys
  DDRB = 0; PORTB = 0xFF;       // Port B all input pullups
  DDRD = 0; PORTD = 0xFF;       // Port D all input pullups
  PCMSK0 = 0xFF; PCMSK2 = 0xFF; // Enable PB0 to PB7 and PD0 to PD7
  PCIFR = 1<<PCIF2 | 1<<PCIF0;  // Clear interrupt flags
  PCICR = 1<<PCIE2 | 1<<PCIE0;  // Enable interrupts
  // Set up INT pin PC7 as INPUT_PULLUP
  IntRelease();
}

int ReadKeys () {
  int Count = 0, Keys, LastKeys = -1;
  // Wait until keys are stable
  do {
    Keys = ~(PINB<<8 | PIND);
    Count++;
    if (LastKeys != Keys) Count = 0;
    LastKeys = Keys;
  } while (Count < 7);
  return Keys;
} 
  
ISR(PCINT0_vect) {
  IntLow();
}

ISR(PCINT2_vect) {
  IntLow();
}

// Light multiplexing **********************************************

void SetupLights () {
  // Set up Timer/Counter1 to multiplex the display
  TCCR1A = 0<<WGM10;            // CTC mode
  TCCR1B = 1<<WGM12 | 2<<CS10;  // CTC mode; divide by 8 = 62500Hz
  OCR1A = 249;                  // Divide by 250 -> 250Hz
  TIMSK1 = TIMSK1 | 1<<OCIE1A;  // Enable compare match interrupt
  // Make LED rows and columns outputs
  DDRC = DDRC | 0x0F;
  DDRA = DDRA | 0x0F;
}

void DisplayNextRow() {
  PORTC = PORTC & ~(0x0F);      // Take rows low
  Row = (Row+1) & 0x03;
  uint8_t Bits = Lights>>(Row*4) & 0x0F;
  PORTA = ~Bits;
  PORTC = PORTC | 1<<Row;       // Take row bit high
}

// Timer/Counter0 interrupt - multiplexes display
ISR(TIMER1_COMPA_vect) {
  DisplayNextRow();
}

// Setup **********************************************

void setup (void) {
  SetupLights();
  SetupKeys();
  // Initialise I2C Slave
  Wire.begin(I2CAddress);
  Wire.onReceive(SetLights);
  Wire.onRequest(GetKeys);
}
 
// Everything under interrupt
void loop (void) {
}
