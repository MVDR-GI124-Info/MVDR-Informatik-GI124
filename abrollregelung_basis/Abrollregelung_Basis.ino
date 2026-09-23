/*
  ============================================================
  MOTORSTEUERUNG MIT ENCODER – KURZANLEITUNG
  ============================================================

  Dieses Programm steuert einen Motor (Richtung + PWM) über einen
  Encoder zur Positions-/Drehzahlrückmeldung. Die Steuerung erfolgt
  über serielle Befehle (Serial Monitor, 9600 Baud).

  BEFEHLSFORMAT:
    <befehl> <wert>

  VERFÜGBARE BEFEHLE:
    rpm <wert>      -> Modus 1: Drehzahlregelung (RPM), wert = Ziel-RPM
    pos <wert>      -> Modus 2: Winkelregelung, wert = Zielwinkel in Grad
    power <wert>    -> Modus 3: Direkte Leistungssteuerung, wert = -100..100
    leng <wert>     -> Modus 4: Seillänge relativ ändern (in cm),
                        wert wird zur aktuellen Seillänge addiert
    setleng <wert>  -> Modus 4: Seillänge absolut setzen (in cm),
                        wert ersetzt die aktuelle Seillänge

  BEISPIELE:
    rpm 50          -> Motor dreht auf 50 RPM
    pos 180         -> Motor fährt auf 180°
    power -75       -> Motor läuft mit 75% Leistung rückwärts
    leng 10         -> Seil wird um 10 cm länger/kürzer gemacht
    setleng 0       -> Seillänge wird auf 0 zurückgesetzt

  HARDWARE:
    - Encoder an Pin 20 und 21
    - Richtungspin: Pin 12
    - PWM-Pin: Pin 3

  ============================================================
*/

#include <Encoder.h>

// Encoder-Objekt, liest die Motorposition über zwei Pins (Quadraturencoder)
Encoder myEnc(20, 21);

// --- Hardware-Konfiguration ---
int directionPin = 12;   // Pin für die Drehrichtung des Motors
int pwmPin = 3;           // Pin für die PWM-Ansteuerung (Motorleistung)

// --- Zustandsvariablen (werden loop-übergreifend genutzt) ---
float pwm = 0;            // aktueller PWM-Wert, der am Motor anliegt
float degree = 0;         // aktueller Winkel des Motors in Grad (aus Encoder berechnet)
float rpm = 0;             // aktuelle gemessene Drehzahl
float targetangle = 0;    // Zielwinkel für Modus 4 (Seillängensteuerung)
float ropelength = 0;     // aktuelle Seillänge
int mode = 0;              // aktueller Betriebsmodus (0 = aus, 1-4 siehe oben)
float input = 0;           // zuletzt empfangener Zahlenwert aus dem seriellen Befehl

void setup() {
  Serial.begin(9600);
  pinMode(directionPin, OUTPUT);
  pinMode(pwmPin, OUTPUT);
}

void loop() {

  // Prüft, ob ein neuer serieller Befehl vorliegt und verarbeitet ihn
  if (Serial.available() > 0) {
    HandelInput();
  }

  // Aktualisiert Position (degree) und Drehzahl (rpm) aus dem Encoder
  mesureRPM();

  // Führt die eigentliche Motorsteuerung je nach aktuellem Modus aus
  Power();
}

// Wählt basierend auf "mode" die passende Steuerfunktion aus
// und übergibt die aktuellen Zustände als Parameter
void Power() {
  switch (mode) {
    case 1:
      setrpm(rpm, input);        // Modus 1: Drehzahlregelung
      break;
    case 2:
      setangle(degree, input);   // Modus 2: Winkelregelung
      break;
    case 3: {
      // Modus 3: direkte Leistungssteuerung ohne Regelung
      float p = constrain(input, -100, 100);  // Eingabe auf -100..100 begrenzen

      // ACHTUNG: hier wird "degree" mit "input" verglichen, nicht targetangle -
      // das ist vermutlich nicht beabsichtigt, aber unverändert übernommen
      digitalWrite(directionPin, degree < input ? HIGH : LOW);

      p = abs(p) * 2.55;   // Prozentwert (0-100) auf PWM-Bereich (0-255) skalieren
      pwm = p;
      Serial.println(pwm);
      analogWrite(pwmPin, pwm);
      break;
    }
    case 4:
      length(degree, targetangle);  // Modus 4: Seillängen-/Positionsregelung
      break;
  }
}

// Fährt den Motor auf den Zielwinkel "target" (für die Seillängensteuerung)
// Nutzt eine quadratische PWM-Rampe: je weiter weg vom Ziel, desto schneller
void length(float currentDegree, float target) {
  float posdif = fabs(currentDegree - target);  // Differenz zum Ziel

  if (posdif < 0.5) {
    // Ziel erreicht (innerhalb Toleranz) -> Motor stoppen
    analogWrite(pwmPin, 0);
  } else {
    // Richtung bestimmen: dreht der Motor vor oder zurück zum Ziel?
    digitalWrite(directionPin, (currentDegree > target) ? HIGH : LOW);

    // PWM steigt quadratisch mit der Entfernung zum Ziel (sanfteres Abbremsen nahe Ziel)
    pwm = constrain(posdif * posdif * 0.001, 0, 255);

    // Mindest-PWM, damit der Motor nicht "hängen bleibt"
    if (pwm < 45) {
      pwm = 45;
    }
    analogWrite(pwmPin, pwm);
  }
  Serial.println(posdif);  // Debug-Ausgabe der aktuellen Abweichung
}

// Regelt die Drehzahl (RPM) über eine einfache proportionale Regelung
void setrpm(float currentRpm, float targetInput) {
  // static: bleibt zwischen Aufrufen erhalten, aber nur innerhalb dieser Funktion sichtbar
  static unsigned long lastPowerUpdate = 0;
  const unsigned long powerInterval = 70;  // Regelintervall in ms

  // Richtung basierend auf Vorzeichen der Zielvorgabe setzen
  digitalWrite(directionPin, degree < targetInput ? HIGH : LOW);

  float targetrpm = abs(targetInput);   // Ziel-RPM (immer positiv, Richtung separat gesetzt)
  float diff = targetrpm - currentRpm;  // Regelabweichung

  // PWM nur alle "powerInterval" ms nachregeln, nicht jeden Loop-Durchlauf
  if (millis() - lastPowerUpdate >= powerInterval) {
    lastPowerUpdate = millis();

    // Größere Abweichung -> stärkere Korrektur, kleine Abweichung -> feine Korrektur
    if (abs(diff) > 2) {
      pwm = pwm + (0.08 * diff);
    } else {
      pwm = pwm + (0.01 * diff);
    }
  }

  pwm = constrain(pwm, 0, 255);  // PWM auf gültigen Bereich begrenzen
  analogWrite(pwmPin, pwm);
}

// Fährt den Motor auf einen Zielwinkel "targetInput" (Modus 2)
// PWM wird stufenweise nach Entfernung zum Ziel gewählt (kein weicher Übergang)
void setangle(float currentDegree, float targetInput) {
  float posdif = fabs(targetInput - currentDegree);  // Abstand zum Zielwinkel

  if (posdif < 1.0) {
    // Innerhalb der Toleranz -> Motor anhalten
    analogWrite(pwmPin, 0);
  } else {
    // Richtung zum Ziel bestimmen
    digitalWrite(directionPin, currentDegree < targetInput ? HIGH : LOW);

    // Stufenweise PWM-Wahl: je weiter weg, desto höher die Geschwindigkeit
    if (posdif >= 100)      pwm = 200;
    else if (posdif >= 40)  pwm = 120;
    else if (posdif >= 20)  pwm = 60;
    else if (posdif >= 8)   pwm = 30;
    else if (posdif >= 3)   pwm = 15;
    else                    pwm = 8;

    analogWrite(pwmPin, pwm);
  }
}

// Liest einen seriellen Befehl ein, parst ihn und setzt Modus + Zielwerte
void HandelInput() {
  String inputString = Serial.readStringUntil('\n');  // Zeile bis Zeilenumbruch lesen
  inputString.trim();  // führende/folgende Leerzeichen entfernen

  int spaceIndex = inputString.indexOf(' ');  // Trennt Befehl und Wert

  if (spaceIndex != -1) {
    String command = inputString.substring(0, spaceIndex);       // z.B. "rpm"
    String valuePart = inputString.substring(spaceIndex + 1);    // z.B. "50"

    input = valuePart.toFloat();  // Wert-Teil in Zahl umwandeln

    if (command == "rpm") {
      mode = 1;
    } else if (command == "pos") {
      mode = 2;
    } else if (command == "power") {
      mode = 3;
    } else if (command == "leng") {
      // Seillänge relativ ändern
      mode = 4;
      input = input * -1;  // Vorzeichen umkehren (Konvention: positiver Input = kürzer)
      ropelength = ropelength + input;
      // Zielwinkel aus Längenänderung berechnen (Umfang = π * Trommeldurchmesser 5)
      targetangle = degree + (input / (PI * 5) * 360.0);
    } else if (command == "setleng") {
      // Seillänge absolut setzen
      mode = 4;
      input = input * -1;
      ropelength = input;
      targetangle = (input / (PI * 5) * 360.0);
    }

  } else {
    Serial.println("Fehler falsches Format");
  }
}

// Berechnet aktuellen Winkel (degree) und Drehzahl (rpm) aus dem Encoder
// Wird in jedem loop()-Durchlauf aufgerufen
void mesureRPM() {
  // static: merkt sich Werte zwischen den Aufrufen, nur lokal in dieser Funktion sichtbar
  static long previoustime = 0;
  static long previousposition = 0;
  static unsigned long lastMeasureAttempt = 0;
  const unsigned long measureInterval = 70;  // Zeitfenster, nach dem RPM=0 gesetzt wird

  long newPosition = myEnc.read() * -1;  // Encoderwert lesen (Vorzeichen invertiert)
  long time = millis();

  if (newPosition != previousposition) {
    // Position hat sich geändert -> RPM und Winkel neu berechnen
    long timeDelta = time - previoustime;
    long posDelta = newPosition - previousposition;

    degree = (float)newPosition / 720.0 * 360.0;  // Encoder-Ticks (720/Umdrehung) in Grad umrechnen

    // RPM = (Umdrehungen) / (Zeit in Minuten)
    rpm = (posDelta / 720.0) / (timeDelta / 60000.0);
    rpm = abs(round(rpm));

    Serial.println(pwm);  // Debug-Ausgabe (Hinweis: zeigt PWM vom vorherigen Loop-Durchlauf)

    previoustime = time;
    previousposition = newPosition;

  } else if (time - lastMeasureAttempt >= measureInterval) {
    // Keine Bewegung seit "measureInterval" ms -> RPM auf 0 setzen (Motor steht)
    lastMeasureAttempt = time;
    rpm = 0;
  }
}
```