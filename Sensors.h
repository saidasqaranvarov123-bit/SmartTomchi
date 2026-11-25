#pragma once
#include "Config.h"
#include "Calibration.h"
#if ST_DHT_MODEL
#include <DHT.h>
#include <math.h>
#endif

struct Reading {
  uint16_t raw = 0;
  int16_t percent = -1;
  int16_t temperature10 = 0;
  int16_t airHumidity10 = 0;
  bool soilValid = false;
  bool airValid = false;
  bool waterAvailable = true;
  bool hasSample = false;
  uint32_t sampledAt = 0;
};

class Sensors {
 public:
  Reading value;
  void begin(bool hardwareReady) {
    ready = hardwareReady;
    if (cfg::demo || !ready) return;
    pinMode(cfg::soilPin, INPUT);
#if ST_DHT_MODEL
    air.begin();
#endif
#if ST_TANK_ENABLED
    pinMode(cfg::tankPin, INPUT_PULLUP);
#endif
  }
  void tick(uint32_t now, const Calibration &calibration) {
    if (value.hasSample && !logic::elapsed(now, value.sampledAt, cfg::sampleMs)) return;
    value.sampledAt = now;
    value.hasSample = true;
    if (cfg::demo) {
      const uint8_t phase = (now / cfg::sampleMs) % 48;
      value.percent = phase <= 24 ? 20 + phase * 3 : 92 - (phase - 24) * 3;
      value.raw = 800 - value.percent * 450L / 100;
      value.temperature10 = 240 + (phase % 8) * 3;
      value.airHumidity10 = 620 + (phase % 7) * 5;
      value.soilValid = true;
      value.airValid = true;
      return;
    }
    value.soilValid = false;
    value.airValid = false;
    value.percent = -1;
    if (!ready) return;

    uint16_t samples[9];
    uint8_t bad = 0;
    analogRead(cfg::soilPin); // ADC kanalini joylashtirish uchun birinchi o'qish.
    for (uint8_t i = 0; i < 9; ++i) {
      samples[i] = analogRead(cfg::soilPin);
      if (!logic::rawHealthy(samples[i])) ++bad;
      for (uint8_t j = i; j > 0 && samples[j] < samples[j - 1]; --j) {
        uint16_t swap = samples[j]; samples[j] = samples[j - 1]; samples[j - 1] = swap;
      }
    }
    value.raw = samples[4];
    value.soilValid = bad < 3 && logic::rawHealthy(value.raw);
    if (value.soilValid && calibration.ready())
      value.percent = logic::moisture(value.raw, calibration.dry, calibration.wet);

#if ST_DHT_MODEL
    const float humidity = air.readHumidity();
    const float temperature = air.readTemperature();
    value.airValid = !isnan(humidity) && !isnan(temperature) &&
                    humidity >= 0 && humidity <= 100 &&
                    temperature >= -40 && temperature <= 80;
    if (value.airValid) {
      value.temperature10 = static_cast<int16_t>(round(temperature * 10));
      value.airHumidity10 = static_cast<int16_t>(round(humidity * 10));
    }
#endif
#if ST_TANK_ENABLED
    value.waterAvailable = digitalRead(cfg::tankPin) == LOW;
#endif
  }
 private:
  bool ready = false;
#if ST_DHT_MODEL
  DHT air{cfg::dhtPin, ST_DHT_MODEL};
#endif
};
