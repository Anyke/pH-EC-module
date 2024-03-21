#include "phec.h"
#include "Arduino.h"
#include "Wire.h"

PhEcModule::PhEcModule(uint8_t ec_addr, uint8_t ph_addr) {
  ecAddr = ec_addr;
  phAddr = ph_addr;
}

bool PhEcModule::readModule() {
  uint8_t buff[23];
  memset(buff, 0, 23);

  wasPhRead = false;

  Wire.begin(); // инициализация I2C
  Wire.requestFrom(phAddr, 20);
  Wire.readBytes(buff, 20);
  Wire.endTransmission();

  if (buff[0] == 0x50 && buff[1] == 0x01) { // сигнатура pH модуля
    //Serial.println("correct signature ph");
    int16_t u = ((int16_t)buff[5] * 256) + (uint16_t)buff[4]; // напряжение в мВ, умноженное на 10
    u /= 10;
    phRawU = u;

    uint16_t ph100 = ((uint16_t)buff[3] * 256) + (uint16_t)buff[2]; // расчитанный pH, умноженный на 100
    pH = (float)ph100 / 100.0f;

    phCal[0] = ((uint16_t)buff[9] * 256) + (uint16_t)buff[8];
    phCal[1] = ((uint16_t)buff[11] * 256) + (uint16_t)buff[10];

    uCal[0] = ((int16_t)buff[13] * 256) + (uint16_t)buff[12];
    uCal[1] = ((int16_t)buff[15] * 256) + (uint16_t)buff[14];
    wasPhRead = true;
  }

  memset(buff, 0, 23);
  wasEcRead = false;

  Wire.begin(); // инициализация I2C
  Wire.requestFrom(ecAddr, 23);      // 0x34 - адрес, 24 - кол-во байт
  Wire.readBytes(buff, 23);
  Wire.endTransmission();

  if (buff[0] == 0x51 && buff[1] == 0x01) { // сигнатура EC модуля
    //Serial.println("correct signature ec");
    EC = ((uint16_t)buff[3] * 256) + (uint16_t)buff[2];
    uint16_t temp100 = ((uint16_t)buff[9] * 256) + (uint16_t)buff[8]; // температура, умноженная на 100. Значение меньше или равно 100 - ошибка чтения датчика температуры
    lvlRawU = ((uint16_t)buff[11] * 256) + (uint16_t)buff[10];        // значение со входа АЦП для датчика давления, если распаян усилитель
    ecRawR = (float)(((int16_t)buff[5] * 256) + (uint16_t)buff[4]);   // "сырое" значение измеренного на переменном токе сопротивления, из которого рассчитывается ЕС

    ecCal[0] = ((uint16_t)buff[13] * 256) + (uint16_t)buff[12];
    ecCal[1] = ((uint16_t)buff[15] * 256) + (uint16_t)buff[14];

    rCal[0] = ((uint16_t)buff[17] * 256) + (uint16_t)buff[16];
    rCal[1] = ((uint16_t)buff[19] * 256) + (uint16_t)buff[18];

    t = (temp100 > 100) ? (float)temp100 / 100.0f : TEMP_ERROR;

    volume = lvlVolCal[0] + (float)((lvlRawU - lvlUCal[0]) * (lvlVolCal[1] - lvlVolCal[0])) / (float)(lvlUCal[1] - lvlUCal[0]);
    wasEcRead = true;
  }
  return (wasEcRead && wasPhRead);
}

bool PhEcModule::calibratePh(float pH1, int16_t mV1, float pH2, int16_t mV2) {
  uint8_t tryCount = 4;
  while ((!this->wasPhRead) && tryCount-- > 0) {
    if (readModule())
      tryCount = 0;
  }

  if (!this->wasPhRead)
    return false;

  int16_t u1m = mV1 * 100, u2m = mV2 * 100;
  uint16_t ph1m = pH1 * 100, ph2m = pH2 * 100;
  if (phCal[0] != ph1m || phCal[1] != ph2m || uCal[0] != u1m || uCal[1] != u2m) {
    tryCount = 4;
    while (tryCount-- > 0) {
      Wire.begin();
      Wire.beginTransmission(phAddr);
      Wire.write(8);
      Wire.write(ph1m & 0xFF);
      Wire.write((ph1m / 256) & 0xFF);
      Wire.write(ph2m & 0xFF);
      Wire.write((ph2m / 256) & 0xFF);
      Wire.write(u1m & 0xFF);
      Wire.write((u1m / 256) & 0xFF);
      Wire.write(u2m & 0xFF);
      Wire.write((u2m / 256) & 0xFF);
      if (Wire.endTransmission(true) == 0)
        return true;
    }
  }
  return false;
}

bool PhEcModule::calibrateEc(uint16_t ec1, uint16_t r1, uint16_t ec2, uint16_t r2) {
  uint8_t tryCount = 4;
  while ((!this->wasEcRead) && tryCount-- > 0) {
    if (readModule())
      tryCount = 0;
  }

  if (!this->wasEcRead)
    return false;

  if (ecCal[0] != ec1 || ecCal[1] != ec2 || rCal[0] != r1 || rCal[1] != r2) {
    tryCount = 4;
    while (tryCount-- > 0) {
      Wire.begin();
      Wire.beginTransmission(ecAddr);
      Wire.write(12);
      Wire.write(ec1 & 0xFF);
      Wire.write((ec1 / 256) & 0xFF);
      Wire.write(ec2 & 0xFF);
      Wire.write((ec2 / 256) & 0xFF);
      Wire.write(r1 & 0xFF);
      Wire.write((r1 / 256) & 0xFF);
      Wire.write(r2 & 0xFF);
      Wire.write((r2 / 256) & 0xFF);
      Wire.write(0x00);
      Wire.write(0x00);
      Wire.write(0xC8); // temp coeff = 0.02 (write 200, 200d = C8h)
      if (Wire.endTransmission(true) == 0)
        return true;
    }
  }
  return false;
}

bool PhEcModule::calibrateVolume(uint32_t vol1, uint16_t u1, uint32_t vol2, uint16_t u2) {
}
void PhEcModule::getCalPh(float *pH1, int16_t *mV1, float *pH2, int16_t *mV2) {
  *pH1 = (float)phCal[0] / 100.0f;
  *pH2 = (float)phCal[1] / 100.0f;
  *mV1 = uCal[0] / 100;
  *mV2 = uCal[1] / 100;
}
void PhEcModule::getCalEc(uint16_t *ec1, uint16_t *r1, uint16_t *ec2, uint16_t *r2) {
  *ec1 = ecCal[0];
  *ec2 = ecCal[1];
  *r1 = rCal[0];
  *r2 = rCal[1];
}
void PhEcModule::getCalVolume(uint32_t *vol1, uint16_t *u1, uint32_t *vol2, uint16_t *u2) {
  *vol1 = lvlVolCal[0];
  *vol2 = lvlVolCal[1];
  *u1 = lvlUCal[0];
  *u2 = lvlUCal[1];
}
