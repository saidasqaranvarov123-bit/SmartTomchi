#pragma once
#include "Config.h"

inline bool addPin(uint8_t pin, uint8_t *used, uint8_t &count, bool analog = false) {
  // Klassik Nano: D0/D1 USB uchun; A6/A7 faqat analog kirish.
  if (pin < 2 || pin > (analog ? A7 : A5) || (analog && pin < A0)) return false;
  for (uint8_t i = 0; i < count; ++i) if (used[i] == pin) return false;
  used[count++] = pin;
  return true;
}

inline bool validatePins() {
  uint8_t used[16];
  uint8_t count = 0;
  bool ok = true;
#if ST_LCD_ENABLED
  ok &= addPin(cfg::lcdRs, used, count);
  ok &= addPin(cfg::lcdEn, used, count);
  ok &= addPin(cfg::lcdD4, used, count);
  ok &= addPin(cfg::lcdD5, used, count);
  ok &= addPin(cfg::lcdD6, used, count);
  ok &= addPin(cfg::lcdD7, used, count);
#endif
#if ST_LEDS_ENABLED
  ok &= addPin(cfg::redPin, used, count);
  ok &= addPin(cfg::greenPin, used, count);
  ok &= addPin(cfg::bluePin, used, count);
#endif
  if (!cfg::demo) {
    ok &= addPin(cfg::soilPin, used, count, true);
#if ST_DHT_MODEL
    ok &= addPin(cfg::dhtPin, used, count);
#endif
#if ST_RELAY_ENABLED
    ok &= addPin(cfg::relayPin, used, count);
#endif
#if ST_TANK_ENABLED
    ok &= addPin(cfg::tankPin, used, count);
#endif
  }
  return ok;
}
