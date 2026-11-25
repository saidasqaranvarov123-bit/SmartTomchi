#pragma once
#include "Sensors.h"

class Controller {
 public:
  bool automatic = false;
  bool running = false;
  bool timeoutFault = false;
  uint32_t totalRunMs = 0;

  void begin(bool hardwareReady, uint32_t now) {
    ready = hardwareReady;
    stoppedAt = now;
#if ST_RELAY_ENABLED
    if (ready && !cfg::demo) {
      digitalWrite(cfg::relayPin, cfg::relayActiveLow ? HIGH : LOW);
      pinMode(cfg::relayPin, OUTPUT);
    }
#endif
#if ST_LEDS_ENABLED
    if (ready) {
      digitalWrite(cfg::redPin, LOW); pinMode(cfg::redPin, OUTPUT);
      digitalWrite(cfg::greenPin, LOW); pinMode(cfg::greenPin, OUTPUT);
      digitalWrite(cfg::bluePin, LOW); pinMode(cfg::bluePin, OUTPUT);
    }
#endif
  }
  bool enable(const Reading &r, const Calibration &calibration, uint32_t now) {
    if (!healthy(r, calibration, now) || timeoutFault) return false;
    automatic = true;
    return true;
  }
  void disable(uint32_t now) {
    automatic = false;
    stop(now);
  }
  void clearFault(uint32_t now) {
    disable(now);
    timeoutFault = false;
  }
  void tick(const Reading &r, const Calibration &calibration, uint32_t now) {
    const bool permitted = automatic && healthy(r, calibration, now) && !timeoutFault;
    if (running && logic::elapsed(now, startedAt, cfg::maxPumpMs)) {
      timeoutFault = true;
      automatic = false;
      stop(now);
      Serial.println(F("XATO: nasos vaqt chegarasi. RESET, keyin AUTO ON kerak."));
    } else if (logic::shouldStop(permitted, running, r.percent, cfg::wetPercent)) {
      stop(now);
    } else if (logic::shouldStart(permitted, running, timeoutFault,
               logic::elapsed(now, stoppedAt, cfg::pumpRestMs),
               r.percent, cfg::dryPercent)) {
      running = true;
      startedAt = now;
      writeRelay(true);
    }
    // Suv/datchik xatosidan keyin avtomatik qayta yoqilmaydi.
    if (automatic && !healthy(r, calibration, now)) automatic = false;
    updateLeds(r, now);
  }
  bool healthy(const Reading &r, const Calibration &calibration, uint32_t now) const {
    return ST_RELAY_ENABLED && ready && !cfg::demo && calibration.ready() &&
           r.hasSample && r.soilValid && r.percent >= 0 && r.waterAvailable &&
           !logic::elapsed(now, r.sampledAt, cfg::staleMs);
  }
  uint32_t runtime(uint32_t now) const {
    return totalRunMs + (running ? static_cast<uint32_t>(now - startedAt) : 0);
  }
 private:
  bool ready = false;
  uint32_t startedAt = 0;
  uint32_t stoppedAt = 0;
  void writeRelay(bool on) {
#if ST_RELAY_ENABLED
    if (ready && !cfg::demo)
      digitalWrite(cfg::relayPin, (on != cfg::relayActiveLow) ? HIGH : LOW);
#else
    (void)on;
#endif
  }
  void stop(uint32_t now) {
    if (!running) return;
    totalRunMs += static_cast<uint32_t>(now - startedAt);
    running = false;
    stoppedAt = now;
    writeRelay(false);
  }
  void updateLeds(const Reading &r, uint32_t now) {
#if ST_LEDS_ENABLED
    if (!ready) return;
    const bool fault = timeoutFault || !r.soilValid || !r.waterAvailable;
    digitalWrite(cfg::redPin, fault ? (now / 400) % 2 :
                 (r.percent >= 0 && r.percent <= cfg::dryPercent));
    digitalWrite(cfg::greenPin, !fault && r.percent > cfg::dryPercent &&
                 r.percent < cfg::wetWarningPercent);
    digitalWrite(cfg::bluePin, running || (!fault && r.percent >= cfg::wetWarningPercent));
#else
    (void)r;
    (void)now;
#endif
  }
};
