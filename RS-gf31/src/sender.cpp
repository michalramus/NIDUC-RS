#include <Arduino.h>
#include "rs.hpp"

double x[6] = {0, 1, 2, 3, 4, 5};
double y[6] = {2, 1, -4, -7, -2, 17};

void transmitCorrect() {
  Serial.println("CORRECT");
  for (int i = 0; i < 6; i++) {
    Serial.print(x[i]); Serial.print(","); Serial.println(y[i]);
  }
}

void transmitOneError() {
  Serial.println("ONE_ERROR");
  double corruptedY[6] = {2, 1, -4, -7, 10, 17};  // np. błąd w punkcie 4
  for (int i = 0; i < 6; i++) {
    Serial.print(x[i]); Serial.print(","); Serial.println(corruptedY[i]);
  }
}

void transmitMoreErrors() {
  Serial.println("MULTI_ERROR");
  double corruptedY[6] = {2, 1, 8, -7, 10, 17};
  for (int i = 0; i < 6; i++) {
    Serial.print(x[i]); Serial.print(","); Serial.println(corruptedY[i]);
  }
}

void setup() {
  Serial.begin(115200);
  delay(2000);


}

void loop() {
  //Serial.println("=== SENDER START ===");
  delay(3000);
  transmitCorrect();
  delay(3000);
  transmitOneError();
  delay(3000);
  transmitMoreErrors();
}
