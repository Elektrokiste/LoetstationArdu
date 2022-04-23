
//MyHT16K33_Bar.cpp
//Code fuer Arduino und Attiny45/85
//Author Retian
//Version 1.3

/*
Ansteuerung einer 4-stelligen 7-Segment-Anzeige mit HT16K33-Chip ueber I2C-Bus

MyHT16K33_Bar Name(I2C-Adresse);

Beispiel siehe unter:
http://arduino-projekte.webnode.at/meine-libraries/7-segm-anz-ht16k33/

Funktionen siehe unter:
http://arduino-projekte.webnode.at/meine-libraries/7-segm-anz-ht16k33/funktionen/

*/

//***************************************************************************
//***************************************************************************

#include "Arduino.h"
#include "MyHT16K33_BAR.h"

MyHT16K33_Bar::MyHT16K33_Bar(byte i2cAdd)
{
  _i2cAdd = i2cAdd;
  
  Wire.begin();
}

//*************************************************************************
//7-Segmengt-Anzeige vorhanden ?

bool MyHT16K33_Bar::isReady()
{
  Wire.beginTransmission(_i2cAdd);
  if (Wire.endTransmission() == 0) return true;
  else return false;
}

//*************************************************************************
//Initialisierung der Anzeige

void MyHT16K33_Bar::init()
{
  sendCmd(HT16K33_OUTPUT_PIN_ROW);
  setOperationMode(HT16K33_DISPLAY_NORMAL);
  setDisplayMode(HT16K33_DISPLAY_ON);
}

//*************************************************************************
//7-Segmengt-Anzeige Helligkeit einstellen

void MyHT16K33_Bar::setBrightness(byte brightness)
{
  byte _brightness = brightness;
 
  if (_brightness >= 0 && _brightness <= 15)
  {
   sendCmd(HT16K33_DIMMING_SET | _brightness);
  }
}

//*************************************************************************
//7-Segmengt-Anzeige dunkel stellen

void MyHT16K33_Bar::sendBlank()
{
  digitT = 0;
  digitH = 0;
  digitZ = 0;
  digitE = 0;
  colon = DISPLAY_COL_OFF;
  decPoint = DISPLAY_DEC_POINT_OFF;
  writeDigits();
  isBlank = true;
}

//*************************************************************************
//Setzen des Display-Modus

void MyHT16K33_Bar::setDisplayMode(byte mode)
{
  byte _mode = mode;
  
  if (_mode >= B10000000 && _mode <= B10000111) sendCmd(_mode);
}

//*************************************************************************
//Setzen des Operation-Modus

void MyHT16K33_Bar::setOperationMode(byte mode)
{
  byte _mode = mode;
  
  if (_mode >= B100000 && _mode <= B100001) sendCmd(_mode);
}

//*************************************************************************
//7-Segment-Anzeige Status dunkel

boolean MyHT16K33_Bar::statusBlank()
{
  return(isBlank);
}

//*************************************************************************
//7-Segment-Anzeige Test

void MyHT16K33_Bar::test7Seg()
{
  digitT = char_8;
  digitH = char_8;
  digitZ = char_8;
  digitE = char_8;
  colon = DISPLAY_COL_ON;
  decPoint = DISPLAY_DEC_POINT_ON;
  writeDigits();
  delay(testTime);
  sendBlank();
}

//*************************************************************************
//Ausgabe einer Fliesskommazahl

void MyHT16K33_Bar::sendFloatVal(float ausgabeWert)
{
  float _ausgabeWert = ausgabeWert;
  boolean kleinerNull;
  char string[5];

  //Pruefe ob _ausgabeWert im Anzeigebereich
  if (_ausgabeWert > 9999 || _ausgabeWert < -999)
  {
    strcpy(string, "oFL ");
    sendString(string);
    return;
  }

  if (_ausgabeWert < 0)
  {
    kleinerNull = true;
    _ausgabeWert *= (-1);
  }
  else kleinerNull = false;


  if (kleinerNull == false)
  {
    if (_ausgabeWert >= 1000)
    {
      _ausgabeWert = _ausgabeWert + 0.5; //+0.5 zur Rundung
      decPoint = DISPLAY_DEC_POINT_OFF;
    }
    else if (_ausgabeWert >= 100)
    {
      _ausgabeWert = (_ausgabeWert * 10) + 0.5; //+0.5 zur Rundung
      decPoint = DISPLAY_DEC_POINT_DIG1;
    }
    else if (_ausgabeWert >= 10)
    {
      _ausgabeWert = (_ausgabeWert * 100) + 0.5; //+0.5 zur Rundung
      decPoint = DISPLAY_DEC_POINT_DIG2;
    }
    else if (_ausgabeWert < 10)
    {
      _ausgabeWert = (_ausgabeWert * 1000) + 0.5; //+0.5 zur Rundung
      decPoint = DISPLAY_DEC_POINT_DIG3;
    }
  }
  
  else if (kleinerNull == true)
  {
    if (_ausgabeWert >= 100)
    {
      _ausgabeWert = (_ausgabeWert * 1) + 0.5; //+0.5 zur Rundung
      decPoint = DISPLAY_DEC_POINT_OFF;
    }
    else if (_ausgabeWert >= 10)
    {
      _ausgabeWert = (_ausgabeWert * 10) + 0.5; //+0.5 zur Rundung
      decPoint = DISPLAY_DEC_POINT_DIG1;
    }
    else if (_ausgabeWert < 10)
    {
      _ausgabeWert = (_ausgabeWert * 100) + 0.5; //+0.5 zur Rundung
      decPoint = DISPLAY_DEC_POINT_DIG2;
    }
  }
  
  //Freistellen 1000er-Stelle
  digitT = _ausgabeWert / 1000;
  //Freistellen 100er-Stelle
  digitH = (_ausgabeWert / 100) - (digitT * 10);
  //Freistellen 10er-Stelle
  digitZ = (_ausgabeWert / 10) - (digitT * 100) - (digitH * 10);
  //Freistellen 1er-Stelle
  digitE = _ausgabeWert - (digitT * 1000) - (digitH * 100) - (digitZ * 10);
  
  colon = DISPLAY_COL_OFF;
  if (kleinerNull == true) digitT = char_Minus;
  else digitT = convertTo7SegCode(digitT);
  digitH = convertTo7SegCode(digitH);
  digitZ = convertTo7SegCode(digitZ);
  digitE = convertTo7SegCode(digitE);
  writeDigits();
  }

//*************************************************************************
//Ausgabe einer Festkommazahl

void MyHT16K33_Bar::sendFixedVal(int ausgabeWert, byte anzNachkomma, bool blankZero)
{
  int _ausgabeWert = ausgabeWert;
  byte _anzNachkomma = anzNachkomma;
  boolean _blankZero = blankZero;
  boolean kleinerNull;
  char string[5];
  
  //Pruefe ob _ausgabeWert im Anzeigebereich
  if (_ausgabeWert > 9999 || _ausgabeWert < -999)
  {
    strcpy(string, "oFL ");
    sendString(string);
    return;
  }
  
  decPoint = DISPLAY_DEC_POINT_OFF;
  
  switch (_anzNachkomma)
  {
    case 0:
      break;
    case 1:
      decPoint = DISPLAY_DEC_POINT_DIG1;
      break;
    case 2:
      decPoint = DISPLAY_DEC_POINT_DIG2;
      break;
    case 3:
      decPoint = DISPLAY_DEC_POINT_DIG3;
      break;
  }

  if (_ausgabeWert < 0)
  {
    kleinerNull = true;
    _ausgabeWert *= (-1);
  }
  else kleinerNull = false;

  //Freistellen 1000er-Stelle
  digitT = _ausgabeWert / 1000;
  //Freistellen 100er-Stelle
  digitH = (_ausgabeWert / 100) - (digitT * 10);
  //Freistellen 10er-Stelle
  digitZ = (_ausgabeWert / 10) - (digitT * 100) - (digitH * 10);
  //Freistellen 1er-Stelle
  digitE = _ausgabeWert - (digitT * 1000) - (digitH * 100) - (digitZ * 10);
  
  //Vornullenunterdrueckung
  if (_blankZero == true)
  {
    if (digitT == 0 && decPoint < DISPLAY_DEC_POINT_DIG3)
    {
      digitT = ' ';
      if (digitH == 0 && decPoint < DISPLAY_DEC_POINT_DIG2)
      {
        digitH = ' ';
        if (digitZ == 0 && decPoint < DISPLAY_DEC_POINT_DIG1)
        {
          digitZ = ' ';
        }
      }
    }
  }
  
  colon = DISPLAY_COL_OFF;
  if (kleinerNull == true) digitT = char_Minus;
  else digitT = convertTo7SegCode(digitT);
  digitH = convertTo7SegCode(digitH);
  digitZ = convertTo7SegCode(digitZ);
  digitE = convertTo7SegCode(digitE);

  writeDigits();
}

//*************************************************************************
//Ausgabe einzelner Digits

void MyHT16K33_Bar::sendDigit(byte digT, byte digH, byte digZ, byte digE, byte dec)
{
  byte _digT = digT;
  byte _digH = digH;
  byte _digZ = digZ;
  byte _digE = digE;
  byte _dec = dec;

  decPoint = DISPLAY_DEC_POINT_OFF;
  switch (_dec)
  {
    case 0:
      break;
    case 1:
      decPoint = DISPLAY_DEC_POINT_DIG0;
      break;
    case 2:
      decPoint = DISPLAY_DEC_POINT_DIG1;
      break;
    case 3:
      decPoint = DISPLAY_DEC_POINT_DIG2;
      break;
    case 4:
      decPoint = DISPLAY_DEC_POINT_DIG3;
      break;
  }
  
  colon = DISPLAY_COL_OFF;
  digitT = convertTo7SegCode(_digT);
  digitH = convertTo7SegCode(_digH);
  digitZ = convertTo7SegCode(_digZ);
  digitE = convertTo7SegCode(_digE);
  writeDigits();
}

//*************************************************************************
//Ausgabe Uhrzeitformat

void MyHT16K33_Bar::sendTime(byte ausgabeWertLinks, byte ausgabeWertRechts)
{
  byte _ausgabeWertLinks = ausgabeWertLinks;
  byte _ausgabeWertRechts = ausgabeWertRechts;

  int _ausgabeWert = _ausgabeWertLinks * 100 + _ausgabeWertRechts;
  
  //Freistellen 1000er-Stelle
  digitT = _ausgabeWert / 1000;
  //Freistellen 100er-Stelle
  digitH = (_ausgabeWert / 100) - (digitT * 10);
  //Freistellen 10er-Stelle
  digitZ = (_ausgabeWert / 10) - (digitT * 100) - (digitH * 10);
  //Freistellen 1er-Stelle
  digitE = _ausgabeWert - (digitT * 1000) - (digitH * 100) - (digitZ * 10);
  
  digitT = convertTo7SegCode(digitT);
  digitH = convertTo7SegCode(digitH);
  digitZ = convertTo7SegCode(digitZ);
  digitE = convertTo7SegCode(digitE);
  colon = DISPLAY_COL_ON;
  decPoint = DISPLAY_DEC_POINT_OFF;
  writeDigits();
}

//*************************************************************************
//Ausgabe von einzelnen Zeichen

void MyHT16K33_Bar::sendChar(char charT, char charH, char charZ, char charE)
{
  char _charT = charT;
  char _charH = charH;
  char _charZ = charZ;
  char _charE = charE;
  
  digitT = convertTo7SegCode(_charT);
  digitH = convertTo7SegCode(_charH);
  digitZ = convertTo7SegCode(_charZ);
  digitE = convertTo7SegCode(_charE);
  decPoint = DISPLAY_DEC_POINT_OFF;
  colon = DISPLAY_COL_OFF;

  writeDigits();
}

//*************************************************************************
//Ausgabe eines Zeichenstringes

void MyHT16K33_Bar::sendString(char* str)
{
  char* _str = str;
  
  digitT = convertTo7SegCode(_str[0]);
  digitH = convertTo7SegCode(_str[1]);
  digitZ = convertTo7SegCode(_str[2]);
  digitE = convertTo7SegCode(_str[3]);

  colon = DISPLAY_COL_OFF;
  decPoint = DISPLAY_DEC_POINT_OFF;
  writeDigits();
}

//*************************************************************************
//Senden eines Kommandos zum HT16K33
//Interne Verwendung

void MyHT16K33_Bar::sendCmd(byte cmd)
{
  Wire.beginTransmission(_i2cAdd);
  Wire.write(cmd);
  Wire.endTransmission();
}

//*************************************************************************
//Schreiben der Digits auf den I2C-Bus
//Interne Verwendung

void MyHT16K33_Bar::writeDigits()
{
  Wire.beginTransmission(_i2cAdd);
  Wire.write(HT16K33_DISPLAY_DATA_ADDRESS_POINTER);
  Wire.write(digitT | (bitRead(decPoint, 3) << 7));
  Wire.write(0);
  Wire.write(digitH | (bitRead(decPoint, 2) << 7));
  Wire.write(0);
  Wire.write(colon);
  Wire.write(0);
  Wire.write(digitZ | (bitRead(decPoint, 1) << 7));
  Wire.write(0);
  Wire.write(digitE | (bitRead(decPoint, 0) << 7));
  Wire.write(0);
  Wire.endTransmission();
  isBlank = false;
}


/*
  Schreibe Bargraph
*/


void MyHT16K33_Bar::sendBarGraph(uint32_t Value)
{
  //0x000F0000 

  Wire.beginTransmission(_i2cAdd);
  Wire.write(HT16K33_DISPLAY_DATA_ADDRESS_POINTER);
  Wire.write((Value & 0xFF));
  Wire.write((Value & 0xFF00)>>8);
  Wire.write((Value & 0x000F0000) >> 16);
  //Wire.write((Value & 0xF0000)>>16);
  
  
  Wire.endTransmission();
  isBlank = false;
}

void MyHT16K33_Bar::writeBarGraph(uint8_t ValueToWrite){
  uint32_t BarGraphData = 0b11111111111111111111 << (20 - constrain(ValueToWrite,0,20));
  sendBarGraph(BarGraphData);

}



//*************************************************************************
//Konvertierung in 7-Segment-Code
//Interne Verwendung

byte MyHT16K33_Bar::convertTo7SegCode(byte val)
{
  byte digit = char_Blank; //Leerzeichen, wenn Zeichen nicht darstellbar
  
  //Umwandlung von Zahlen in Ascii-Code
  if (val >= 0 && val < 10) val += 48;
  
  switch (val)
  {
    case '0':
      digit = char_0;
      break;
    case '1':
      digit = char_1;
      break;
    case '2':
      digit = char_2;
      break;
    case '3':
      digit = char_3;
      break;
    case '4':
      digit = char_4;
      break;
    case '5':
      digit = char_5;
      break;
    case '6':
      digit = char_6;
      break;
    case '7':
      digit = char_7;
      break;
    case '8':
      digit = char_8;
      break;
    case '9':
      digit = char_9;
      break;
      case ' ':
      digit = char_Blank;
      break;
    case '-':
      digit = char_Minus;
      break;
    case 'A':
      digit = char_A;
      break;
    case 'a':
      digit = char_A;
      break;
    case 'B':
      digit = char_b;
      break;
    case 'b':
      digit = char_b;
      break;
    case 'C':
      digit = char_C;
      break;
    case 'c':
      digit = char_c;
      break;
    case 'D':
      digit = char_d;
      break;
    case 'd':
      digit = char_d;
      break;
    case 'E':
      digit = char_E;
      break;
    case 'e':
      digit = char_e;
      break;
    case 'F':
      digit = char_F;
      break;
    case 'f':
      digit = char_F;
      break;
    case 'G':
      digit = char_G;
      break;
    case 'g':
      digit = char_G;
      break;
    case 'H':
      digit = char_H;
      break;
    case 'h':
      digit = char_h;
      break;
    case 'I':
      digit = char_I;
      break;
    case 'i':
      digit = char_i;
      break;
    case 'J':
      digit = char_J;
      break;
    case 'j':
      digit = char_J;
      break;
    case 'L':
      digit = char_L;
      break;
    case 'l':
      digit = char_l;
      break;
    case 'N':
      digit = char_n;
      break;
    case 'n':
      digit = char_n;
      break;
    case 'O':
      digit = char_O;
      break;
    case 'o':
      digit = char_o;
      break;
    case 'P':
      digit = char_P;
      break;
    case 'p':
      digit = char_P;
      break;
    case 'Q':
      digit = char_q;
      break;
    case 'q':
      digit = char_q;
      break;
    case 'R':
      digit = char_r;
      break;
    case 'r':
      digit = char_r;
      break;
    case 'S':
      digit = char_S;
      break;
    case 's':
      digit = char_S;
      break;
    case 'T':
      digit = char_t;
      break;
    case 't':
      digit = char_t;
      break;
    case 'U':
      digit = char_U;
      break;
    case 'u':
      digit = char_u;
      break;
    case 'Y':
      digit = char_Y;
      break;
    case 'y':
      digit = char_Y;
      break;
  }
  return digit;
}



