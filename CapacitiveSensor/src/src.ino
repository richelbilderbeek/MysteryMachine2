  /*
  MysteryMachine2
  (C) 2015 Richel Bilderbeek

*/
/*

Description chip SN74HC595:

 16 15 14 13 12 11 10 9
 |  |  |  |  |  |  |  |
 +--+--+--+--+--+--+--+
 |>                   |
 +--+--+--+--+--+--+--+
 |  |  |  |  |  |  |  |
 1  2  3  4  5  6  7  8

 Official pin names:

 1: Q2
 2: Q3
 3: Q4
 3: Q5
 5: Q6
 6: Q7
 7: Q8 
 8: GND
 9: QH*: for daisy chaining, unused otherwise
 10: SRCLR* (to 5V)
 11: SRCLK (to clock pin)
 12: RCLK (to latch pin)
 13: OE* (to GND)
 14: SER (to data pin)
 15: Q1
 16: VCC (naar 5V)


First shift register:

 16 15 14 13 12 11 10 9
 |  |  |  |  |  |  |  |
 +--+--+--+--+--+--+--+
 |>                   |
 +--+--+--+--+--+--+--+
 |  |  |  |  |  |  |  |
 1  2  3  4  5  6  7  8

 Verbindingen:

 1: to LED_M2
 2: to LED_M3
 3: LED 4
 3: LED 5
 5: LED 6
 6: LED 7
 7: LED 8 
 8: GND
 9: naar pin 14 van het tweede shift register (dit is de daisy chain verbinding)
 10: naar 5V
 11: naar D4: clock pin
 12: naar D3: latch pin
 13: naar GND
 14: naar D2: data pin
 15: to LED_M1
 16: naar 5V

Tweede shift register:

 16 15 14 13 12 11 10 9
 |  |  |  |  |  |  |  |
 +--+--+--+--+--+--+--+
 |>                   |
 +--+--+--+--+--+--+--+
 |  |  |  |  |  |  |  |
 1  2  3  4  5  6  7  8

 Verbindingen:

 1: LED 10
 2: LED 11
 3: LED 12
 3: LED 13
 5: LED 14
 6: LED 15
 7: LED 816
 8: GND
 9: wordt niet gebruikt
 10: naar 5V
 11: naar D4: clock pin
 12: naar D3: latch pin
 13: naar GND
 14: naar pin 9 van eerste shift register (dit is de daisy chain verbinding)
 15: naar LED 9
 16: naar 5V

  Connecting the three sensors:
  
  6              7 
  |  +--------+  |
  +--+ R      +--+
  |  +--------+
  |
  X
  
  8              9 
  |  +--------+  |
  +--+ R      +--+
  |  +--------+
  |
  X

  10             11 
  |  +--------+  |
  +--+ R      +--+
  |  +--------+
  |
  X

  6,8,10: sensor pins
  7,9,11: helper pins
  R: resistance of at least 1 Mega-Ohm (brown-black-green-gold)
  X: place to touch wire

*/

#include <CapacitiveSensor.h>

const int datapin  = 2;
const int latchpin = 3;
const int clockpin = 4;

const int pin_sensor1 =  6;
const int pin_helper1 =  7;
CapacitiveSensor sensor1 
  = CapacitiveSensor(pin_helper1,pin_sensor1);        
const int pin_sensor2 =  8;
const int pin_helper2 =  9;
CapacitiveSensor sensor2 
  = CapacitiveSensor(pin_helper2,pin_sensor2);        

const int pin_sensor3 = 10;
const int pin_helper3 = 11;
CapacitiveSensor sensor3 
  = CapacitiveSensor(pin_helper3,pin_sensor3);        

//Which of player 1's LEDs is active?
int p1 = 0;
//Which of player 2's LEDs is active?
int p2 = 0;

const bool debug = true;

void setup() 
{
  Serial.begin(9600);
  pinMode(latchpin, OUTPUT);
  pinMode(clockpin, OUTPUT);
  pinMode(datapin , OUTPUT);
}

void loop() 
{
  //The higher 'samples' is set, the more accurate the sensors measure
  const int samples = 30;

  //Measure the capacitive sensors
  const int r1 = sensor1.capacitiveSensor(samples);
  const int r2 = sensor2.capacitiveSensor(samples);
  const int r3 = sensor3.capacitiveSensor(samples);

  if (debug)
  {
    //Show the values in the Serial Monitor
    Serial.print(r1);
    Serial.print(" ");
    Serial.print(r2);
    Serial.print(" ");
    Serial.println(r3);
  }

  //The threshold value, which determines the sensitivity of the sensors
  // - too low: the program will think more often there is a touch, possibly even when you do not touch
  // - too high: the program will think less often there is a touch, possibly even when you do touch 
  const int t = 100;
  
  //Respond to touches
  if (r1 >= t) { p1 = (p1 + 1) % 5; }
  if (r2 >= t) { p2 = (p2 + 1) % 5; }
  const bool mid_pressed = r3 >= t;
  const int pm = CalcMidPin(p1,p2);

  ShowState(p1,p2,mid_pressed ? pm : -1);

  delay(100);
}

//0: mid pin at side 1
//1: mid pin at center
//2: mid pin at side 2
int CalcMidPin(const int p1, const int p2)
{
  if (p1 == p2) return 1;
  return (10 + p1 - p2) % 2 == 0 ? 0 : 2;
}

//s1: side 1 state
//s2: side 2 state
//mid: mid state, if mid == -1, then no center LED will be on
void ShowState(const int s1, const int s2, const int mid)
{
  const int b1 = 1 << s1;  
  const int b2 = 1 << s2;  
  //if mid == -1, then no center LED will be on
  const int bm = mid >= 0 ? 1 << mid : 0;
  const int value = (b2 << 8) + (b1 << 3) + bm;
  ShowBinary(value);
}

void ShowBinary(const int value) 
{
  //Start met schrijven
  digitalWrite(latchpin,LOW);

  //Bereken wat te schrijven
  // (dit moet in twee keer: een keer per shift register)
  const int high_value = value / 256;
  const int low_value  = value % 256;

  //Schrijf naar de shift registers
  //(opmerking: mocht je toch maar een shift register aansluiten,
  // dan wordt de high_value gewoon overschreven door de low_value,
  // oftewel: je ziet dan enkel de low_value)
  WriteToShiftRegister(high_value); 
  WriteToShiftRegister(low_value);

  //Klaar met schrijven
  digitalWrite(latchpin,HIGH);
}

void WriteToShiftRegister(const byte value) 
{
  // This shifts 8 bits out MSB first, 
  //on the rising edge of the clock,
  //clock idles low

  //prepare shift register for bit shifting
  digitalWrite(datapin, LOW);
  digitalWrite(clockpin, LOW);

  for (int i=7; i>=0; i--)  
  {
    digitalWrite(clockpin, 0);

    //if the value passed to myDataOut and a bitmask result 
    // true then... so if we are at i=6 and our value is
    // %11010100 it would the code compares it to %01000000 
    // and proceeds to set pinState to 1.
    //Sets the pin to HIGH or LOW depending on pinState
    digitalWrite(datapin, value & (1<<i) ? HIGH : LOW);
    //register shifts bits on upstroke of clock pin  
    digitalWrite(clockpin, HIGH);
    //zero the data pin after shift to prevent bleed through
    digitalWrite(datapin, LOW);
  }

  //stop shifting
  digitalWrite(clockpin, LOW);
}
