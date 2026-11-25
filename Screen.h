#pragma once
#include <stdio.h>
#include <string.h>
#include "Controller.h"
#if ST_LCD_ENABLED
#include <LiquidCrystal.h>
#endif

class Screen {
 public:
  void begin(bool hardwareReady) {
    ready = hardwareReady;
    memset(previous, 0, sizeof(previous));
#if ST_LCD_ENABLED
    if (ready) {
      lcd.begin(16, 2);
      for (uint8_t width = 1; width <= 5; ++width) {
        uint8_t glyph[8];
        for (uint8_t i = 0; i < 8; ++i) glyph[i] = (0x1F << (5 - width)) & 0x1F;
        lcd.createChar(width, glyph);
      }
    }
#endif
  }
  void tick(uint32_t now, const Reading &r, const Calibration &cal,
            const Controller &control, bool pinsValid) {
    if (drawn && !logic::elapsed(now, lastDraw, cfg::screenMs)) return;
    drawn = true;
    lastDraw = now;
    if (now >= cfg::bootMs) bootComplete = true;
    memset(rows, ' ', sizeof(rows));
    rows[0][16] = rows[1][16] = '\0';
    if (!pinsValid) {
      text(0, F("SOZLAMA XATOSI")); text(1, F("Pinlar takror"));
    } else if (!cfg::demo && !ready) {
      text(0, F("SMART TOMCHI")); text(1, F("PINLARNI SOZLANG"));
    } else if (!bootComplete) {
      text(0, F("SMART TOMCHI"));
      text(1, cfg::demo ? F("DEMO yuklanmoqda") : F("Ishga tushmoqda"));
      rows[0][15] = "|/-\\"[(now / 250) % 4];
    } else if (!cfg::demo && control.timeoutFault) {
      text(0, F("NASOS TO'XTADI")); text(1, F("USB: RESET"));
    } else if (!cfg::demo && !r.waterAvailable) {
      text(0, F("SUV YO'Q")); text(1, F("Idishni tekshiring"));
    } else if (!cfg::demo && (!r.hasSample || !r.soilValid)) {
      text(0, F("DATCHIK XATOSI"));
      snprintf_P(rows[1], 17, PSTR("RAW:%4u TEKSHIR"), r.raw);
    } else if (!cfg::demo && logic::elapsed(now, r.sampledAt, cfg::staleMs)) {
      text(0, F("MALUMOT ESKIRDI")); text(1, F("NASOS O'CHIRILDI"));
    } else if (!cfg::demo && !cal.ready()) {
      text(0, F("KALIBRLASH KERAK"));
      snprintf_P(rows[1], 17, PSTR("RAW:%4u USB CAL"), r.raw);
    } else {
      const uint8_t page = ((now - cfg::bootMs) / cfg::pageMs) % 5;
      renderPage(page, now, r, control);
    }
    commit();
  }
 private:
  bool ready = false;
  bool drawn = false;
  bool bootComplete = false;
  uint32_t lastDraw = 0;
  char rows[2][17];
  char previous[2][17];
#if ST_LCD_ENABLED
  LiquidCrystal lcd{cfg::lcdRs, cfg::lcdEn, cfg::lcdD4,
                    cfg::lcdD5, cfg::lcdD6, cfg::lcdD7};
#endif
  void text(uint8_t row, const __FlashStringHelper *message) {
    const char *source = reinterpret_cast<const char *>(message);
    for (uint8_t i = 0; i < 16; ++i) {
      const char c = pgm_read_byte(source + i);
      if (!c) break;
      rows[row][i] = c;
    }
  }
  void renderPage(uint8_t page, uint32_t now, const Reading &r, const Controller &c) {
    switch (page) {
      case 0: {
        snprintf_P(rows[0], 17, PSTR("%sNamlik:%3d%%"), cfg::demo ? "DEMO " : "", r.percent);
        rows[1][0] = '['; rows[1][11] = ']';
        const uint8_t filled = r.percent * 50L / 100;
        for (uint8_t i = 0; i < 10; ++i) {
          const int8_t partial = static_cast<int16_t>(filled) - i * 5;
          rows[1][i + 1] = partial <= 0 ? ' ' : (partial >= 5 ? 5 : partial);
        }
        const char *state = r.percent <= cfg::dryPercent ? "QUR" :
                            (r.percent >= cfg::wetWarningPercent ? "NAM" : "OK ");
        memcpy(rows[1] + 13, state, 3);
        break;
      }
      case 1:
        text(0, cfg::demo ? F("DEMO Havo") : F("Harorat / havo"));
        if (r.airValid) {
          const int16_t absolute = abs(r.temperature10);
          snprintf_P(rows[1], 17, PSTR("T:%s%d.%dC H:%d%%"),
            r.temperature10 < 0 ? "-" : "", absolute / 10,
            absolute % 10, r.airHumidity10 / 10);
        } else text(1, ST_DHT_MODEL ? F("DHT XATOSI") : F("DHT ulanmagan"));
        break;
      case 2:
        text(0, cfg::demo ? F("DEMO Tahlil") : F("Namlik tahlili"));
        text(1, r.percent <= cfg::dryPercent ? F("QURUQ: suv kerak") :
          (r.percent >= cfg::wetWarningPercent ? F("NAM: sug'ormang") : F("ME'YOR: holat OK")));
        break;
      case 3:
        text(0, cfg::demo ? F("DEMO Sug'orish") : F("Sug'orish"));
        text(1, cfg::demo ? F("DEMO: nasos OFF") :
          (!ST_RELAY_ENABLED ? F("Rele ulanmagan") :
          (c.running ? F("NASOS ISHLAYAPTI") :
          (c.automatic ? F("AUTO: kutilmoqda") : F("AUTO o'chirilgan")))));
        break;
      default:
        text(0, cfg::demo ? F("DEMO Tizim") : F("SMART TOMCHI"));
        snprintf_P(rows[1], 17, PSTR("RAW:%4u %5lus"), r.raw, now / 1000UL);
        break;
    }
  }
  void commit() {
    bool changed = false;
    // Qator to'liq yoziladi, lcd.clear() siklda ishlatilmaydi.
    for (uint8_t row = 0; row < 2; ++row) {
      bool ended = false;
      for (uint8_t col = 0; col < 16; ++col) {
        if (rows[row][col] == '\0') ended = true;
        if (ended) rows[row][col] = ' ';
      }
      if (memcmp(rows[row], previous[row], 16) == 0) continue;
      changed = true;
#if ST_LCD_ENABLED
      if (ready) {
        lcd.setCursor(0, row);
        for (uint8_t col = 0; col < 16; ++col) lcd.write(rows[row][col]);
      }
#endif
      memcpy(previous[row], rows[row], 17);
    }
    if (changed) {
      Serial.println(F("+----------------+"));
      for (uint8_t row = 0; row < 2; ++row) {
        Serial.write('|');
        for (uint8_t col = 0; col < 16; ++col) {
          const char c = rows[row][col];
          Serial.write(c >= 1 && c <= 5 ? '#' : c);
        }
        Serial.println('|');
      }
      Serial.println(F("+----------------+"));
    }
  }
};
