// Compile with: real mode, confirmed pins, relay enabled, -I<repository>.
#include "Config.h"
#include "LogicChecks.h"
#include "Hardware.h"
#include "Controller.h"

uint8_t failures = 0;
uint8_t checks = 0;
void check(bool passed, const __FlashStringHelper *name) {
  ++checks;
  Serial.print(passed ? F("PASS ") : F("FAIL "));
  Serial.println(name);
  if (!passed) ++failures;
}
void fresh(Reading &r, uint32_t now, int16_t percent) {
  r.hasSample = true; r.soilValid = true; r.waterAvailable = true;
  r.sampledAt = now; r.percent = percent;
}
void setup() {
  Serial.begin(9600);
  check(validatePins(), F("default pin mapping"));
  uint8_t used[16]; uint8_t count = 0;
  check(addPin(3, used, count) && !addPin(3, used, count), F("duplicate pin rejected"));
  check(!addPin(0, used, count), F("UART pin rejected"));
  check(!addPin(A6, used, count), F("analog-only output rejected"));
  Calibration cal;
  check(!cal.ready(), F("uncalibrated startup"));
  check(cal.capture(true, 800) && cal.capture(false, 350) && cal.ready(), F("two-point calibration"));
  check(cal.save(), F("EEPROM write and verify"));
  Calibration loaded; loaded.begin();
  check(loaded.ready() && loaded.dry == 800 && loaded.wet == 350, F("EEPROM reload"));
  EEPROM.update(4, EEPROM.read(4) ^ 1);
  Calibration corrupt; corrupt.begin();
  check(!corrupt.ready(), F("EEPROM checksum rejection"));

  Controller c; Reading r;
  c.begin(true, 0);
  check(!c.running && !c.automatic && digitalRead(cfg::relayPin) == HIGH, F("relay off at boot"));
  fresh(r, 100, 20);
  check(c.enable(r, cal, 100), F("AUTO enabled with valid input"));
  c.tick(r, cal, 100);
  check(!c.running, F("startup rest period"));
  fresh(r, 30000, 20); c.tick(r, cal, 30000);
  check(c.running && digitalRead(cfg::relayPin) == LOW, F("dry soil starts relay"));
  fresh(r, 33000, 40); c.tick(r, cal, 33000);
  check(c.running, F("hysteresis keeps watering"));
  fresh(r, 34000, 55); c.tick(r, cal, 34000);
  check(!c.running && c.automatic, F("wet threshold stops watering"));
  fresh(r, 35000, 20); c.tick(r, cal, 35000);
  check(!c.running, F("restart rest period"));
  fresh(r, 64000, 20); c.tick(r, cal, 64000);
  check(c.running, F("start after rest"));
  fresh(r, 74000, 20); c.tick(r, cal, 74000);
  check(!c.running && c.timeoutFault && !c.automatic, F("timeout latches fault"));
  check(!c.enable(r, cal, 74000), F("fault blocks AUTO"));
  c.clearFault(75000);
  check(!c.timeoutFault && !c.automatic, F("RESET does not enable AUTO"));
  fresh(r, 104000, 20); c.enable(r, cal, 104000); c.tick(r, cal, 104000);
  c.tick(r, cal, 110501);
  check(!c.running && !c.automatic, F("stale sample stops relay"));
  fresh(r, 141000, 20); c.enable(r, cal, 141000); c.tick(r, cal, 141000);
  r.waterAvailable = false; c.tick(r, cal, 141001);
  check(!c.running && !c.automatic, F("empty tank stops relay"));
  fresh(r, 172000, 20); c.enable(r, cal, 172000); c.tick(r, cal, 172000);
  r.soilValid = false; c.tick(r, cal, 172001);
  check(!c.running && !c.automatic, F("invalid sensor stops relay"));
  fresh(r, 200000, 20);
  check(!c.enable(r, corrupt, 200000), F("uncalibrated AUTO rejected"));
  Controller blocked; blocked.begin(false, 0);
  check(!blocked.enable(r, cal, 200000), F("unconfirmed hardware blocked"));
  Sensors sensors; sensors.begin(true); sensors.tick(0, cal);
  check(sensors.value.soilValid && sensors.value.raw >= 510 && sensors.value.raw <= 513,
        F("actual ADC acquisition and median"));
  check(sensors.value.percent >= 63 && sensors.value.percent <= 65, F("ADC to calibrated percent"));
  Serial.print(F("CHECKS=")); Serial.println(checks);
  Serial.println(failures ? F("TESTS_FAILED") : F("ALL_TESTS_PASSED"));
}
void loop() {}
