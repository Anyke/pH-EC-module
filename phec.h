#ifndef PHEC_SENSOR_H
#define PHEC_SENSOR_H
#include "stdint.h"

const float TEMP_ERROR = 0.0f;
const uint32_t VOLUME_ERROR = 0;

class PhEcModule {
public:
  //PhEcModule();
  PhEcModule(uint8_t ec_addr = 0x34, uint8_t ph_addr = 0x33);
  bool readModule();

  float getTemperature() { return t; }
  uint16_t getEc() { return EC; }
  float getPh() { return pH; }
  //bool getLvl1State();
  //bool getLvl2State();
  uint32_t getVolumeML() { return volume; }
  uint16_t getRawVolumeU() { return lvlRawU; }
  int getRawEcR() { return ecRawR; }
  int getRawPhU() { return phRawU; }

  bool calibratePh(float pH1, int16_t mV1, float pH2, int16_t mV2);
  bool calibrateEc(uint16_t ec1, uint16_t r1, uint16_t ec2, uint16_t r2);
  bool calibrateVolume(uint32_t vol1, uint16_t u1, uint32_t vol2, uint16_t u2);
  void getCalPh(float *pH1, int16_t *mV1, float *pH2, int16_t *mV2);
  void getCalEc(uint16_t *ec1, uint16_t *r1, uint16_t *ec2, uint16_t *r2);
  void getCalVolume(uint32_t *vol1, uint16_t *u1, uint32_t *vol2, uint16_t *u2);
  //bool calibrateEcTempSensor();

private:
  
  uint8_t phAddr, ecAddr;

  bool wasPhRead = false;
  bool wasEcRead = false;

  float t;
  float pH;
  uint16_t EC;
  uint32_t volume;
  uint32_t phRawU;
  uint32_t ecRawR;
  uint16_t lvlRawU;

  uint16_t phCal[2];
  int16_t uCal[2];
  uint16_t ecCal[2];
  uint16_t rCal[2];

  uint32_t lvlVolCal[2];
  uint16_t lvlUCal[2];
};

#endif
