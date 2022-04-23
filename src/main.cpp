#include <Arduino.h>

// Bibliothek für die PID Regelung
#include <PID_v1.h>

// Bibliothek um die 4x7Segment Anzeigen anzusteuern
#include <MyHT16K33_7Seg.h>

// Bibliothek um die BarGraph Anzeige anzusteuern
#include <MyHT16K33_BAR.h>

// Bibliothek um den Encoder auszulesen
#include <Encoder.h>

// Bibliothek um die Verwendung des TimerInterrupts verstaendlicher zu machen
#define TIMER_INTERRUPT_DEBUG         0
#define _TIMERINTERRUPT_LOGLEVEL_     0

#define USE_TIMER_1     true
#include "TimerInterrupt.h"

HT16K33 DigitDisplay(0b11100000);

// Erstelle ein Encoder Object; der Encoder ist an Pin 2 und 3 angeschlossen
Encoder myEnc(2, 3);

// Deklariere 3 Variablen, die die Schnittstelle zur PID-Berechnung stellen
double Sollwert, Input, Output;

// Definiere die Reglerbeiwerte
double Kp=2, Ki=5, Kd=1;
// Erstelle ein PID Okject mit den definierten Schnittstellenvariablen und den Reglerbeiwerten;
// mit DIRECT wird der Regler auf nicht-invertiernd gestellt
PID myPID(&Input, &Output, &Sollwert, Kp, Ki, Kd, DIRECT);

bool LEDState = 0;

// Definiere eine Funktion, die vom Timerinterrupt ausgeführt werden soll
void TimerInterruptFunktion() {
  digitalWrite(13,!LEDState);
  LEDState = !LEDState;
}


void setup() {
  // Initialisiere den TimerInterrupt, 10ms Wartezeit und der Funktion, die aufgerufen werden soll,
  // wenn der Timer auslöst
  Serial.begin(9600);
  pinMode(13,OUTPUT);
  DigitDisplay.Init();

  myEnc.write(325);

  ITimer1.init();
  ITimer1.attachInterruptInterval(100, TimerInterruptFunktion);
}
long oldPosition  = -999;

void loop() {
  //Serial.println("Test");
  Serial.println(analogRead(0));
 
  int32_t Encoderposition = myEnc.read();
  Sollwert = constrain(Encoderposition,100,400);;
  if (Encoderposition < 100 || Encoderposition > 400){
    myEnc.write(Sollwert);
  }
  //Serial.println(Sollwert);
 
}


