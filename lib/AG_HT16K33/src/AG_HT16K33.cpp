#if ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#include <AG_HT16K33.h>
#include <Wire.h>


#define DDAP 0b0 //Display Data Adress Pointer, die vier LSBs geben die Speicherstelle an
#define SSR 0x20 // System Setup Register
#define KDAP 0x40 // Key Data Address Pointer
#define INTFAP 0x60 // INT Flag Address Pointer
#define DSR 0x80 // Display Setup Register
#define RIS 0xA0 // ROW / INT set
#define DIM 0xEF // Dimming set


HT16K33::HT16K33(int Adresse){
    Address = Adresse;

}

void HT16K33::Init(){
    Wire.begin();
    Wire.beginTransmission(Address);
    Wire.write(byte(SSR)| 0x1);
    Wire.endTransmission();

    Wire.beginTransmission(Address);
    Wire.write(byte(RIS) | 0b0000);
    Wire.endTransmission();
    Wire.beginTransmission(Address);
    Wire.write(byte(RIS) | 0b0100);
    Wire.endTransmission();
    Wire.beginTransmission(Address);
    Wire.write(byte(RIS) | 0b1000);
    Wire.endTransmission();
    Wire.beginTransmission(Address);
    Wire.write(byte(RIS) | 0b1100);
    Wire.endTransmission();

    Wire.beginTransmission(Address);
    Wire.write(byte(DIM) | 0b0111);
    Wire.endTransmission();

    Wire.beginTransmission(Address);
    Wire.write(byte(DSR) | 0b00000011);
    Wire.endTransmission();
}

void HT16K33::WriteToDisplay(){
    delay(100);

}