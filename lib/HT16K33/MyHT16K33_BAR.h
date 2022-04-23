//MyHT16K33_BAR.h

#ifndef MyHT16K33_BAR_h
#define MyHT16K33_BAR_h

#if defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__) || defined (__AVR_ATtiny84__)
  #include "TinyWireM.h"
  #define Wire TinyWireM
#else
  #include <Wire.h>
#endif

#include "Arduino.h"

#define HT16K33_DISPLAY_DATA_ADDRESS_POINTER 0x00
#define HT16K33_SYSTEM_SETUP                 0x20
#define HT16K33_DISPLAY_SETUP                0x80
#define HT16K33_ROW_INT_SET                  0xA0
#define HT16K33_DIMMING_SET                  0xE0

#define HT16K33_DISPLAY_STANDBY              0x20
#define HT16K33_DISPLAY_NORMAL               0x21

#define HT16K33_DISPLAY_OFF                  0x80
#define HT16K33_DISPLAY_ON                   0x81
#define HT16K33_DISPLAY_BLINK_1              0x85
#define HT16K33_DISPLAY_BLINK_2              0x83
#define HT16K33_DISPLAY_BLINK_0p5            0x87

#define HT16K33_OUTPUT_PIN_ROW               0xA0
#define HT16K33_OUTPUT_PIN_INT_ACTIV_LOW     0xA1
#define HT16K33_OUTPUT_PIN_INT_ACTIV_HIGH    0xA3

#define DISPLAY_COL_ON  B10 //Doppelpunkt Ein
#define DISPLAY_COL_OFF B00 //Doppelpunkt Aus
#define DISPLAY_DEC_POINT_OFF  B0000 //Alle Dezimalpunkte Aus
#define DISPLAY_DEC_POINT_DIG0 B0001 //Dezimalpunkt auf Einer-Digit
#define DISPLAY_DEC_POINT_DIG1 B0010 //Dezimalpunkt auf Zehner-Digit
#define DISPLAY_DEC_POINT_DIG2 B0100 //Dezimalpunkt auf Hunderter-Digit
#define DISPLAY_DEC_POINT_DIG3 B1000 //Dezimalpunkt auf Tausender-Digit
#define DISPLAY_DEC_POINT_ON   B1111 //Alle Dezimalpunkte Ein

//7-Segment-Code der darstellbare Zeichen
//7-Segment:   DPgfedcba
#define char_0 B00111111
#define char_1 B00000110
#define char_2 B01011011
#define char_3 B01001111
#define char_4 B01100110
#define char_5 B01101101
#define char_6 B01111101
#define char_7 B00000111
#define char_8 B01111111
#define char_9 B01101111
#define char_Blank B00000000 //' '-Zeichen
#define char_Minus B01000000 //'-' Zeichen
#define char_A B01110111
#define char_b B01111100
#define char_C B00111001
#define char_c B01011000
#define char_d B01011110
#define char_E B01111001
#define char_e B01111011
#define char_F B01110001
#define char_G B00111101
#define char_H B01110110
#define char_h B01110100
#define char_I B00000110
#define char_i B00000100
#define char_J B00001110
#define char_L B00111000
#define char_l B00000110
#define char_n B01010100
#define char_O B00111111
#define char_o B01011100
#define char_P B01110011
#define char_q B01100111
#define char_r B01010000
#define char_S B01101101
#define char_t B01111000
#define char_U B00111110
#define char_u B00011100
#define char_Y B01101110

#define testTime 1000 //Testzeit [ms] der Funktion test7Seg()

class MyHT16K33_Bar
{
  public:
    MyHT16K33_Bar(byte);
    bool isReady(void);
    void init(void);
    void setOperationMode(byte);
    void setDisplayMode(byte);
    void setBrightness(byte);
    void sendBlank(void);
    void sendFloatVal(float);
    void sendFixedVal(int, byte, bool);
    bool statusBlank(void);
    void sendDigit(byte, byte, byte, byte, byte);
    void sendTime(byte, byte);
    void test7Seg();
    void sendChar(char, char, char, char);
    void sendString(char*);
    void sendBarGraph(uint32_t);
    void writeBarGraph(uint8_t);
            
    
  private:
    void sendCmd(byte);
    byte convertTo7SegCode(byte);
    void writeDigits();
	  
    byte _i2cAdd;
    boolean isBlank;
	
    byte digitT = 0;
    byte digitH = 0;
    byte digitZ = 0;
    byte digitE = 0;
    byte colon = 0; //Doppelpunkt
    byte decPoint = 0;
};

#endif

