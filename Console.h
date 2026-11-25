#pragma once
#include <ctype.h>
#include <string.h>
#include "Controller.h"

class Console {
 public:
  void tick(Sensors &sensors, Calibration &calibration, Controller &control, uint32_t now) {
    for (uint8_t budget = 0; budget < 16 && Serial.available(); ++budget) {
      const char c = Serial.read();
      if (c == '\r' || c == '\n') {
        if (overflow) Serial.println(F("XATO: buyruq juda uzun."));
        else if (length) {
          command[length] = '\0';
          execute(sensors, calibration, control, now);
        }
        length = 0; overflow = false;
      } else if (!overflow) {
        if (length >= sizeof(command) - 1) overflow = true;
        else if (c >= 32 && c <= 126) command[length++] = toupper(c);
      }
    }
  }
  void help() const {
    Serial.println(F("Smart Tomchi 1.0 | 9600 baud | Newline"));
    Serial.println(F("HELP, STATUS, RAW, CAL DRY, CAL WET, SAVE"));
    Serial.println(F("AUTO ON, AUTO OFF, RESET (faultni tozalaydi; AUTO OFF qoladi)"));
    Serial.println(F("DEMO qiymatlar haqiqiy o'lchov emas."));
  }
  void status(const Reading &r, const Calibration &cal, const Controller &c, uint32_t now) const {
    Serial.print(cfg::demo ? F("MODE=DEMO") : F("MODE=REAL"));
    Serial.print(F(" RAW=")); Serial.print(r.raw);
    Serial.print(F(" SOIL="));
    if (r.soilValid && r.percent >= 0) Serial.print(r.percent); else Serial.print(F("NA"));
    Serial.print(F(" TEMP="));
    if (r.airValid) Serial.print(r.temperature10 / 10.0, 1); else Serial.print(F("NA"));
    Serial.print(F(" AIR="));
    if (r.airValid) Serial.print(r.airHumidity10 / 10.0, 1); else Serial.print(F("NA"));
    Serial.print(F(" CAL=")); Serial.print(cal.ready() ? 1 : 0);
    Serial.print(F(" DRY=")); Serial.print(cal.dry);
    Serial.print(F(" WET=")); Serial.print(cal.wet);
    Serial.print(F(" WATER="));
    if (ST_TANK_ENABLED && !cfg::demo && cfg::pinsConfirmed) Serial.print(r.waterAvailable ? 1 : 0);
    else Serial.print(F("NA"));
    Serial.print(F(" AUTO=")); Serial.print(c.automatic ? 1 : 0);
    Serial.print(F(" PUMP=")); Serial.print(c.running ? 1 : 0);
    Serial.print(F(" TIMEOUT=")); Serial.print(c.timeoutFault ? 1 : 0);
    Serial.print(F(" RUN_S=")); Serial.println(c.runtime(now) / 1000UL);
  }
 private:
  char command[32];
  uint8_t length = 0;
  bool overflow = false;
  void execute(Sensors &s, Calibration &cal, Controller &c, uint32_t now) {
    const Reading &r = s.value;
    if (!strcmp_P(command, PSTR("HELP"))) help();
    else if (!strcmp_P(command, PSTR("STATUS"))) status(r, cal, c, now);
    else if (!strcmp_P(command, PSTR("RAW"))) {
      Serial.print(cfg::demo ? F("DEMO RAW=") : F("RAW=")); Serial.println(r.raw);
    } else if (!strcmp_P(command, PSTR("AUTO OFF"))) {
      c.disable(now); Serial.println(F("OK: AUTO OFF, nasos o'chirildi."));
    } else if (!strcmp_P(command, PSTR("AUTO ON"))) {
      Serial.println(c.enable(r, cal, now) ? F("OK: AUTO ON. Tanaffus saqlanadi.") :
        F("RAD: pin/rele/datchik/kalibr/suv/fault holatini tekshiring."));
    } else if (!strcmp_P(command, PSTR("RESET"))) {
      c.clearFault(now); Serial.println(F("OK: fault tozalandi. AUTO OFF."));
    } else if (!strcmp_P(command, PSTR("SAVE"))) {
      Serial.println(cal.save() ? F("OK: kalibrlash EEPROM ga saqlandi.") :
        F("RAD: real rejimda ikki nuqta kalibrlanishi kerak."));
    } else if (!strcmp_P(command, PSTR("CAL DRY")) || !strcmp_P(command, PSTR("CAL WET"))) {
      c.disable(now);
      if (!r.hasSample || !r.soilValid || logic::elapsed(now, r.sampledAt, cfg::staleMs) ||
          !cal.capture(!strcmp_P(command, PSTR("CAL DRY")), r.raw)) {
        Serial.println(F("RAD: yangi haqiqiy datchik o'qishi kerak."));
      } else {
        s.value.percent = cal.ready() ? logic::moisture(r.raw, cal.dry, cal.wet) : -1;
        Serial.println(cal.ready() ? F("OK: ikki nuqta tayyor. SAVE yozing.") :
          F("QABUL: ikkinchi nuqtani o'lchang; farq kamida 100 RAW."));
      }
    } else Serial.println(F("Noma'lum buyruq. HELP yozing."));
  }
};
