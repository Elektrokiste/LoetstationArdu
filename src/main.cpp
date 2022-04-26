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
double Sollwert = 325;
double Istwert = 20;
double Output = 0;
double aktuelleTemperatur = 0;
// Definiere die Reglerbeiwerte
//double Kp=2, Ki=5, Kd=1;
double Kp=8, Ki=8, Kd=0;
// Erstelle ein PID Okject mit den definierten Schnittstellenvariablen und den Reglerbeiwerten;
// mit DIRECT wird der Regler auf nicht-invertiernd gestellt
PID myPID(&aktuelleTemperatur, &Output, &Sollwert, Kp, Ki, Kd, DIRECT);

/*
  Erzeuge Anzeige-Objekte und Variablen
*/
MyHT16K33_7Seg SiebSegAnzeige(0x70 | 0b000);  // Da keiner der Jumper A0 bis A2 gesetzt ist, muss die Adresse gleich der Basisadresse sein 
MyHT16K33_Bar BarGraphAnzeige(0x70 | 0b001);  // Da nur A1 gesetzt ist, ist die I2C Adresse um 0b001 höher als die Basis adresse

char DisplayBuffer[4]; // Ziwschenspeicher für die Anzeigedaten

/*
  Erstelle ein Encoder Object
*/
Encoder myEnc(2, 3); //der Encoder ist an Pin 2 und 3 angeschlossen, da diese beiden Pins Interruptfähhig sind
long oldPosition  = -999; // in dieser Variable wird stets der letze Encoderwert gespeichtert, um eine Veränderung zu bemerken

/*
  Erstelle Allgemeine Variablen, die nicht einer Funktionsgruppe zuzuordnen sind
*/
bool LEDState = 0;  // In dieser Variable wird der aktuelle Zustand der LED an Pin 13 gespeichert
long lastEncoderChange = 0; // In dieser Variable wird der letzte millis()-Wert gespeichert, bei dem der Encoder gedreht wurde
long lastTimeSiebSegChange = 0;

#define Mittelwert 100  // Es werden die letzten n Messungen aufaddiert, und anschließend der Mittelwert ermittelt
float TemperaturBuffer[Mittelwert]; // hier werden die letzen #Mittelwert Temperaturmessungen abgespeichert
float Temperaturmittelwert = 0;

/*
  Definiere eine Funktion, die vom Timerinterrupt ausgeführt werden soll
*/
// Hier wird der Regler in einem Festgelegten Zeitintervall aufgerufen, damit die Zeitabhängen I und D Regler funktionieren
void TimerInterruptFunktion() {
  
  // Blinke die LED an Pin 13
  digitalWrite(13,!LEDState);
  LEDState = !LEDState;

  // Temperaturmessung am Thermoelement in der Lötspitze
  digitalWrite(5,LOW);  // deaktiviere die Heizung, damit die Leitungen des Thermoelements weniger Störungen haben, wenn gemessen wird
  delay(5); // Warte 5 Millisekunden, damit sich die Leitungskapazität abbauen kann
  // Verscheibe die Werte im TemperaturBuffer um eine Stelle nach vorne. Also Stelle 10 bekommt den inhalt von Stelle 9 usw
  for (int i = Mittelwert - 1; i >= 1;i--){ 
    TemperaturBuffer[i] = TemperaturBuffer[i - 1];
  }

  // Lies den aktuellen Temperaturwert ein, und rechne den gemessenen Wert in den kalibrierten Messbereich um
  TemperaturBuffer[0] = map(analogRead(0),400,1000,100,400);

  // Rechne den Mittelwert aus den Messwerten im TemperaturBuffer aus
  Temperaturmittelwert = 0;  
  for (int i = 0; i < Mittelwert;i++){
    Temperaturmittelwert += TemperaturBuffer[i];
  }
  aktuelleTemperatur = Temperaturmittelwert / Mittelwert;

  // Berechne den PID-Regler neu, mit dem neuen Temperatur-Ist wert und dem unter umständen geänderten Sollwert
  myPID.Compute();
  
  // Steuere die Leistungselektronik an
  if (analogRead(0) < 1000){  // Wenn eine Lötspitze eingebaut ist (Sonst wird Pin A0 auf HIGH gezogen)
    analogWrite(5,Output);  // Steuere die Leistungselektronik mit dem vom PID-Regler ausgegebenen Wert an
  }else{  // Wenn keine Lötspitze eingebaut ist
    digitalWrite(5,LOW);  // Schalte die Leistungselektronik aus
  }  
}


void setup() {
  // Starte die Serial-Schnittstelle um Fehlersuche zu erleichtern
  Serial.begin(9600);

  // Setze die pinModes der einzelnen Pins
  pinMode(13,OUTPUT);

  // Setze den Standardwert für den Lötkolben auf 325°C; Das Encoderobjekt beinhaltet stets den aktuellen Sollwert
  myEnc.write(325*2);

  // Initialisiere den TimerInterrupt, 10ms Wartezeit und der Funktion, die aufgerufen werden soll,
  // wenn der Timer auslöst
  ITimer1.init();
  ITimer1.attachInterruptInterval(10, TimerInterruptFunktion);

  // Initialisiere die Anzeigen und Zeige ein Testbild an
  BarGraphAnzeige.init();
  BarGraphAnzeige.writeBarGraph(20);
  SiebSegAnzeige.init();
  SiebSegAnzeige.test7Seg();
  BarGraphAnzeige.writeBarGraph(0);

  // Zeige beim Start die Zieltemperatur an
  lastEncoderChange = millis();

  // Initialisiere PID:
  myPID.SetMode(AUTOMATIC);
}


void loop() {  
  // Verarbeite die Encoderposition 
  int32_t Encoderposition = int(myEnc.read()/2); // liest den aktuellen Encoderwert ein
  Sollwert = constrain(Encoderposition,100,400);  // setze die Solltemperatur auf die Encoderposition, begrenze den Wert
  if (Encoderposition < 100 || Encoderposition > 400){  // Überschreibe den Encoderwert wenn er einen Grenzwert überschreitet
    myEnc.write(Sollwert*2);
  }

  if (oldPosition != Sollwert){ // Wenn der Encoderwert geändert wurde
    lastEncoderChange  = millis();  // speichere den aktuellen Zeitpunkt
    oldPosition = Sollwert; // aktualisiere den zwischenspeicher
  }


  // Steuere die Siebensegmentanzeige an
  if (millis() - lastEncoderChange < 1000){ // Wenn der Encoderwert innerhalb der letzen Sekunde verändert wurde
    String TextToShow  = String(Sollwert) + "C";  // Setze einen String aus der gesetzen Temperatur und C zusammen
    TextToShow.toCharArray(DisplayBuffer,4);  // Wandle diesen String in ein char Array um
    SiebSegAnzeige.sendString(DisplayBuffer); // schreibe dieses char Array auf die Siebensegment anzeige
  }else{
    if (millis() - lastTimeSiebSegChange > 100){   // Wenn die Temperaturanzeige zuletzt vor mehr als 100ms aktualisiert wurde
      lastTimeSiebSegChange = millis();
      if (aktuelleTemperatur > 400){  // und die Lötspitzentemperatur höher als 400grad ist 
        SiebSegAnzeige.sendString(" -- ");  // Zeige an, das keine Lötspitze eingebaut ist
      }else{  // Sonst  
        SiebSegAnzeige.sendFixedVal(int(aktuelleTemperatur * 10),1,0);  // Zeige die aktuelle Temperatur mit einer Nachkommastelle an
      }
    }
  }

  // Zeige den PID-Output auf der Bargraph-Anzeige an
  int Graph = int(map(Output,0,255,0,20));
  BarGraphAnzeige.writeBarGraph(Graph);
}


