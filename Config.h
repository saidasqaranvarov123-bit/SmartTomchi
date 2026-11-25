#pragma once
#include <Arduino.h>

// Default: USB namoyish. Rasmdagi pinlar hali tasdiqlanmagan.
// -D compiler flags ham ushbu sozlamalarni almashtira oladi.
#ifndef ST_DEMO_MODE
#define ST_DEMO_MODE 1
#endif
#ifndef ST_PIN_MAPPING_CONFIRMED
#define ST_PIN_MAPPING_CONFIRMED 0
#endif
// 0 = USB ekran nusxasi, 1 = parallel HD44780 16x2 LCD.
#ifndef ST_LCD_ENABLED
#define ST_LCD_ENABLED 0
#endif
// Faqat datchik modeli aniqlangach 11 yoki 22 ni tanlang.
#ifndef ST_DHT_MODEL
#define ST_DHT_MODEL 0
#endif
#ifndef ST_RELAY_ENABLED
#define ST_RELAY_ENABLED 0
#endif
#ifndef ST_LEDS_ENABLED
#define ST_LEDS_ENABLED 0
#endif
#ifndef ST_TANK_ENABLED
#define ST_TANK_ENABLED 0
#endif

namespace cfg {
constexpr bool demo = ST_DEMO_MODE;
constexpr bool pinsConfirmed = ST_PIN_MAPPING_CONFIRMED;
constexpr uint32_t serialBaud = 9600;
constexpr uint32_t sampleMs = 2500;
constexpr uint32_t staleMs = 6500;
constexpr uint32_t screenMs = 250;
constexpr uint32_t pageMs = 4000;
constexpr uint32_t bootMs = 4000;
constexpr uint32_t maxPumpMs = 10000;
constexpr uint32_t pumpRestMs = 30000;
constexpr uint8_t dryPercent = 30;
constexpr uint8_t wetPercent = 55;
constexpr uint8_t wetWarningPercent = 80;
// Faqat namunaviy pinlar. Tayyor qurilma ulanishini tekshirib moslang!
constexpr uint8_t soilPin = A0;
constexpr uint8_t dhtPin = 7;
constexpr uint8_t relayPin = 6;
constexpr bool relayActiveLow = true;
constexpr uint8_t tankPin = A1; // INPUT_PULLUP: HIGH = suv yo'q/sim uzilgan.
constexpr uint8_t redPin = 8;
constexpr uint8_t greenPin = 9;
constexpr uint8_t bluePin = 10;
constexpr uint8_t lcdRs = 12;
constexpr uint8_t lcdEn = 11;
constexpr uint8_t lcdD4 = 5;
constexpr uint8_t lcdD5 = 4;
constexpr uint8_t lcdD6 = 3;
constexpr uint8_t lcdD7 = 2;
constexpr uint16_t defaultDry = 800; // Kalibrlashgacha faqat taxmin.
constexpr uint16_t defaultWet = 350;
}

static_assert(ST_DHT_MODEL == 0 || ST_DHT_MODEL == 11 || ST_DHT_MODEL == 22,
              "ST_DHT_MODEL must be 0, 11 or 22");
static_assert(cfg::dryPercent < cfg::wetPercent && cfg::wetPercent <= 100,
              "Invalid moisture thresholds");
static_assert(cfg::maxPumpMs > 0 && cfg::pumpRestMs >= cfg::maxPumpMs,
              "Invalid pump timing");
