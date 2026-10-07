#include <Arduino.h>
#include <math.h>

// --- Pin-Belegung fuer 4 Taster ---
const int pinUp    = A0;  // Y- (nach oben)
const int pinDown  = A1;  // Y+ (nach unten)
const int pinLeft  = A2;  // X- (nach links)
const int pinRight = A3;  // X+ (nach rechts)

// --- Geometrische Parameter ---
const float W = 100.0;
const float drumDiam = 5.0;
const float stepSize = 1.0;

// --- Koordinaten ---
float posX = 50.0;
float posY = 50.0;

// --- Sollabrollwinkel ---
float aS1 = 0.0;
float aS2 = 0.0;

void calculateKinematics() {
  float L1 = sqrt((posX * posX) + (posY * posY));
  float L2 = sqrt(((W - posX) * (W - posX)) + (posY * posY));

  const float drumCircumference = PI * drumDiam;
  aS1 = (L1 / drumCircumference) * 360.0;
  aS2 = (L2 / drumCircumference) * 360.0;
}

void handleButtons() {
  static unsigned long lastInputTime = 0;
  const unsigned long debounceDelay = 100;

  if (millis() - lastInputTime >= debounceDelay) {
    bool moved = false;

    // LOW oznachaet, chto knopka zamknuta na GND
    if (digitalRead(pinUp) == LOW) {
      posY -= stepSize;
      moved = true;
    }
    if (digitalRead(pinDown) == LOW) {
      posY += stepSize;
      moved = true;
    }
    if (digitalRead(pinLeft) == LOW) {
      posX -= stepSize;
      moved = true;
    }
    if (digitalRead(pinRight) == LOW) {
      posX += stepSize;
      moved = true;
    }

    if (moved) {
      posX = constrain(posX, 5.0, W - 5.0);
      posY = constrain(posY, 5.0, 200.0);

      calculateKinematics();

      Serial.print("X: "); Serial.print(posX);
      Serial.print(" cm | Y: "); Serial.print(posY);
      Serial.print(" cm -> aS1: "); Serial.print(aS1);
      Serial.print(" Grad | aS2: "); Serial.print(aS2);
      Serial.println(" Grad");

      lastInputTime = millis();
    }
  }
}

void setup() {
  Serial.begin(9600);

  // Vstroennaya podtyazhka k 5V cherez mikrokontroller
  pinMode(pinUp, INPUT_PULLUP);
  pinMode(pinDown, INPUT_PULLUP);
  pinMode(pinLeft, INPUT_PULLUP);
  pinMode(pinRight, INPUT_PULLUP);

  calculateKinematics();
  Serial.println("SkyCam Controller bereit!");
}

void loop() {
  handleButtons();
}
