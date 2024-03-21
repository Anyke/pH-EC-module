#include "phec.h"

PhEcModule phec;

void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly
  if (phec.readModule()) {
    Serial.println( (phec.getTemperature() != TEMP_ERROR) ? String(phec.getTemperature(), 1) : String("temperature_error"));
    Serial.println((phec.getEc() > 13 && phec.getEc() < 20000) ? phec.getEc() : 0);
    Serial.println((phec.getPh() > 1.8 && phec.getPh() < 11.0f) ? String(phec.getPh(), 2) : String("ph error, maybe not calibrated"));
  } else {
    Serial.println("read error");
  }

}
