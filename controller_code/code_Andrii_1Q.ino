// --- Pins für 4 Taster (Controller) ---
const int pinUp    = 4;   // Y+
const int pinDown  = 5;   // Y-
const int pinLeft  = 6;   // X-
const int pinRight = 7;   // X+

// --- Geometrische Parameter (an Modell anpassen) ---
const float W = 100.0;       // Abstand zwischen beiden Aufhängepunkten/Winden S1 und S2 in cm
const float drumDiam = 5.0;  // Trommeldurchmesser in cm
const float stepSize = 0.5;  // Schrittweite pro Tastendruck/Zyklus in cm

// --- Aktuelle Position der Sky-Cam ---
float posX = 50.0;  // Startwert X (Mitte)
float posY = 50.0;  // Startwert Y (Tiefe nach unten)

// --- Berechnete Sollwerte ---
float aS1 = 0.0;  // Sollabrollwinkel Winde 1 in Grad
float aS2 = 0.0;  // Sollabrollwinkel Winde 2 in Grad

void setupInputs() {
  // Interne Pullup-Widerstände nutzen (Taster schalten gegen GND)
  pinMode(pinUp, INPUT_PULLUP);
  pinMode(pinDown, INPUT_PULLUP);
  pinMode(pinLeft, INPUT_PULLUP);
  pinMode(pinRight, INPUT_PULLUP);
}

// Berechnet die Seillängen und daraus die Sollabrollwinkel (aS1, aS2)
void calculateKinematics() {
  // Inverse Kinematik (Pythagoras):
  // Winde 1 liegt bei (0, 0) -> L1 = sqrt(x^2 + y^2)
  // Winde 2 liegt bei (W, 0) -> L2 = sqrt((W - x)^2 + y^2)
  float L1 = sqrt((posX * posX) + (posY * posY));
  float L2 = sqrt(((W - posX) * (W - posX)) + (posY * posY));

  // Umrechnung Seillänge in Drehwinkel: Winkel = (Länge / Umfang) * 360°
  const float drumCircumference = PI * drumDiam;
  aS1 = (L1 / drumCircumference) * 360.0;
  aS2 = (L2 / drumCircumference) * 360.0;
}

// Liest die 4 Taster ein und aktualisiert X/Y sowie die Sollwinkel
void handleButtons() {
  static unsigned long lastInputTime = 0;
  const unsigned long debounceDelay = 50;  // Taster-Entprellung / Schrittintervall in ms

  if (millis() - lastInputTime >= debounceDelay) {
    bool moved = false;

    // LOW bedeutet Taste gedrückt (bei INPUT_PULLUP)
    if (digitalRead(pinUp) == LOW) {
      posY -= stepSize;  // Nach oben (Y verringern)
      moved = true;
    }
    if (digitalRead(pinDown) == LOW) {
      posY += stepSize;  // Nach unten (Y vergrößern)
      moved = true;
    }
    if (digitalRead(pinLeft) == LOW) {
      posX -= stepSize;  // Nach links
      moved = true;
    }
    if (digitalRead(pinRight) == LOW) {
      posX += stepSize;  // Nach rechts
      moved = true;
    }

    if (moved) {
      // Grenzen sichern, damit die Kamera im Feld bleibt
      posX = constrain(posX, 5.0, W - 5.0);
      posY = constrain(posY, 5.0, 200.0);

      calculateKinematics();
      lastInputTime = millis();
    }
  }
}
