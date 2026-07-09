#include "wokwi-api.h"
#include <math.h>

#define I2C_ADDRESS 0x57

static uint8_t redValue;
static uint8_t irValue;
static float t = 0;

void chip_init() {
}

bool chip_i2c_write(uint8_t addr, uint8_t *data, int len) {
  return true;
}

bool chip_i2c_read(uint8_t addr, uint8_t *data, int len) {

  t += 0.1;

  redValue = 100 + 20 * sin(t);
  irValue  = 120 + 25 * sin(t);

  if (len >= 2) {
    data[0] = redValue;
    data[1] = irValue;
  }

  return true;
}