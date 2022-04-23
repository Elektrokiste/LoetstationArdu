#include <Arduino.h>

// Bibliothek für die PID Regelung
#include <PID_v1.h>

// Bibliothek um die 4x7Segment Anzeigen anzusteuern
#include <MyHT16K33_7Seg.h>

// Bibliothek um die BarGraph Anzeige anzusteuern
#include <MyHT16K33_BAR.h>

// Bibliothek um den Encoder auszulesen
#include <Encoder.h>

// Bibliothek und Defines um die Verwendung des TimerInterrupts verstaendlicher zu machen
#define TIMER_INTERRUPT_DEBUG         0
#define _TIMERINTERRUPT_LOGLEVEL_     0

#define USE_TIMER_1     true
#include "TimerInterrupt.h"

/*  
  Erzeuge Regler-Variablen
*/
// Deklariere 3 Variablen, die die Schnittstelle zur PID-Berechnung stellen
double Sollwert, Input, Output;
// Definiere die Reglerbeiwerte
double Kp=2, Ki=5, Kd=1;
// Erstelle ein PID Okject mit den definierten Schnittstellenvariablen und den Reglerbeiwerten;
// mit DIRECT wird der Regler auf nicht-invertiernd gestellt
PID myPID(&Input, &Output, &Sollwert, Kp, Ki, Kd, DIRECT);

/*
  Erzeuge Anzeige-Objekte
*/
MyHT16K33_7Seg SiebSegAnzeige(0x70 | 0b000);  // Da keiner der Jumper A0 bis A2 gesetzt ist, muss die Adresse gleich der Basisadresse sein 
MyHT16K33_Bar BarGraphAnzeige(0x70 | 0b001);  // Da nur A1 gesetzt ist, ist die I2C Adresse um 0b001 höher als die Basis adresse

/*
  Erstelle ein Encoder Object
*/
Encoder myEnc(2, 3); //der Encoder ist an Pin 2 und 3 angeschlossen, da diese beiden Pins Interruptfähhig sind
long oldPosition  = -999; // in dieser Variable wird stets der letze Encoderwert gespeichtert, um eine Veränderung zu bemerken

/*
  Erstelle Allgemeine Variablen, die nicht einer Funktionsgruppe zuzuordnen sind
*/
bool LEDState = 0;  // In dieser Variable wird der aktuelle Zustand der LED an Pin 13 gespeichert

/*
  Definiere eine Funktion, die vom Timerinterrupt ausgeführt werden soll
*/
void TimerInterruptFunktion() {
  digitalWrite(13,!LEDState);
  LEDState = !LEDState;
}


void setup() {
  // Starte die Serial-Schnittstelle um Fehlersuche zu erleichtern
  Serial.begin(9600);

  // Setze die pinModes der einzelnen Pins
  pinMode(13,OUTPUT);

  // Setze den Standardwert für den Lötkolben auf 325°C; Das Encoderobjekt beinhaltet stets den aktuellen Sollwert
  myEnc.write(325);

  // Initialisiere den TimerInterrupt, 10ms Wartezeit und der Funktion, die aufgerufen werden soll,
  // wenn der Timer auslöst
  ITimer1.init();
  ITimer1.attachInterruptInterval(100, TimerInterruptFunktion);
}


void loop() {
  // Schreibe den Aktuell gemessenen Wert des Thermoelements auf die Serielle Schnittstelle
  Serial.println(analogRead(0));

  // Verarbeite die Encoderposition 
  int32_t Encoderposition = myEnc.read(); // liest den aktuellen Encoderwert ein
  Sollwert = constrain(Encoderposition,100,400);  // setze die Solltemperatur auf die Encoderposition, begrenze den Wert
  if (Encoderposition < 100 || Encoderposition > 400){  // Überschreibe den Encoderwert wenn er einen Grenzwert überschreitet
    myEnc.write(Sollwert);
  } 
}


