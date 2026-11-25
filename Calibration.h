#pragma once
#include <EEPROM.h>
#include "Logic.h"
#include "Config.h"

struct CalibrationRecord {
  uint16_t magic;
  uint8_t version;
  uint8_t flags;
  uint16_t dry;
  uint16_t wet;
  uint16_t checksum;
};

class Calibration {
 public:
  uint16_t dry = cfg::defaultDry;
  uint16_t wet = cfg::defaultWet;
  uint8_t captured = 0;

  bool ready() const {
    return captured == 3 && logic::calibrationValid(dry, wet);
  }
  void begin() {
    if (cfg::demo || !cfg::pinsConfirmed) return;
    CalibrationRecord record;
    EEPROM.get(0, record);
    if (record.magic == 0x5354 && record.version == 1 && record.flags == 3 &&
        record.checksum == checksum(record) &&
        logic::calibrationValid(record.dry, record.wet)) {
      dry = record.dry;
      wet = record.wet;
      captured = 3;
    }
  }
  bool capture(bool isDry, uint16_t raw) {
    if (cfg::demo || !cfg::pinsConfirmed || !logic::rawHealthy(raw)) return false;
    if (isDry) { dry = raw; captured |= 1; }
    else { wet = raw; captured |= 2; }
    return true;
  }
  bool save() const {
    if (!ready() || cfg::demo || !cfg::pinsConfirmed) return false;
    CalibrationRecord record = {0x5354, 1, 3, dry, wet, 0};
    record.checksum = checksum(record);
    EEPROM.put(0, record); // update semantics: faqat o'zgargan baytlar yoziladi.
    CalibrationRecord verified;
    EEPROM.get(0, verified);
    return verified.magic == record.magic && verified.version == 1 &&
           verified.flags == 3 && verified.dry == dry && verified.wet == wet &&
           verified.checksum == checksum(verified);
  }
 private:
  static uint16_t checksum(const CalibrationRecord &record) {
    uint16_t crc = 0xFFFF;
    const uint8_t *bytes = reinterpret_cast<const uint8_t *>(&record);
    for (uint8_t i = 0; i < 8; ++i) {
      crc ^= bytes[i];
      for (uint8_t bit = 0; bit < 8; ++bit)
        crc = (crc & 1) ? (crc >> 1) ^ 0xA001 : crc >> 1;
    }
    return crc;
  }
};
static_assert(sizeof(CalibrationRecord) == 10, "Unexpected EEPROM record layout");
