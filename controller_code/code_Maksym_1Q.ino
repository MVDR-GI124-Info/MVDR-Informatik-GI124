/*
  ============================================================
  ENCODER-MODUL FÜR ZWEI MOTOREN (Nur Sensor-Auswertung)
  ============================================================
*/
 
#include <Encoder.h>
 
// Encoder-Objekte definieren
Encoder encA(20, 21);
Encoder encB(18, 19);
 
// Mechanische Parameter
const float TICKS_PER_REV = 720.0; // Anpassbar je nach Encoder/Getriebe
 
// Globale Ausgabewerte für die Hauptsteuerung
float degreeA = 0.0;
float degreeB = 0.0;
float rpmA = 0.0;
float rpmB = 0.0;
 
void setupEncoder() {
  // Encoder.h initialisiert die Pins intern automatisch
}
 
void readEncoders() {
  mesureRPM_A();
  mesureRPM_B();
}
 
void mesureRPM_A() {
  static long previoustimeA = 0;
  static long previousposA = 0;
  static unsigned long lastAttemptA = 0;
  const unsigned long measureInterval = 70;
 
  long newPos = encA.read();
  long time = millis();
 
  if (newPos != previousposA) {
    long timeDelta = time - previoustimeA;
    long posDelta = newPos - previousposA;
 
    degreeA = (float)newPos / TICKS_PER_REV * 360.0;
    
    if (timeDelta > 0) {
      rpmA = (posDelta / TICKS_PER_REV) / (timeDelta / 60000.0);
      rpmA = abs(round(rpmA));
    }
    previoustimeA = time;
    previousposA = newPos;
  } else if (time - lastAttemptA >= measureInterval) {
    lastAttemptA = time;
    rpmA = 0;
  }
}
 
void mesureRPM_B() {
  static long previoustimeB = 0;
  static long previousposB = 0;
  static unsigned long lastAttemptB = 0;
  const unsigned long measureInterval = 70;
 
  long newPos = encB.read();
  long time = millis();
 
  if (newPos != previousposB) {
    long timeDelta = time - previoustimeB;
    long posDelta = newPos - previousposB;
 
    degreeB = (float)newPos / TICKS_PER_REV * 360.0;
    
    if (timeDelta > 0) {
      rpmB = (posDelta / TICKS_PER_REV) / (timeDelta / 60000.0);
      rpmB = abs(round(rpmB));
    }
    previoustimeB = time;
    previousposB = newPos;
  } else if (time - lastAttemptB >= measureInterval) {
    lastAttemptB = time;
    rpmB = 0;
  }
}
 
