#pragma once
#include <stdint.h>

namespace logic {
constexpr bool rawHealthy(uint16_t raw) { return raw > 4 && raw < 1019; }
constexpr int32_t span(uint16_t dry, uint16_t wet) {
  return static_cast<int32_t>(wet) - dry;
}
constexpr bool calibrationValid(uint16_t dry, uint16_t wet) {
  return rawHealthy(dry) && rawHealthy(wet) &&
         (span(dry, wet) >= 100 || span(dry, wet) <= -100);
}
constexpr int16_t clampPercent(int32_t value) {
  return value < 0 ? 0 : (value > 100 ? 100 : static_cast<int16_t>(value));
}
// Har ikkala yo'nalishdagi analog datchik uchun ham ishlaydi.
constexpr int16_t moisture(uint16_t raw, uint16_t dry, uint16_t wet) {
  return !calibrationValid(dry, wet) ? -1 :
    clampPercent((static_cast<int32_t>(raw) - dry) * 100 / span(dry, wet));
}
constexpr bool elapsed(uint32_t now, uint32_t then, uint32_t interval) {
  return static_cast<uint32_t>(now - then) >= interval;
}
constexpr bool shouldStart(bool permitted, bool running, bool fault,
                           bool rested, int16_t percent, uint8_t dry) {
  return permitted && !running && !fault && rested &&
         percent >= 0 && percent <= dry;
}
constexpr bool shouldStop(bool permitted, bool running,
                          int16_t percent, uint8_t wet) {
  return running && (!permitted || percent < 0 || percent >= wet);
}
}
