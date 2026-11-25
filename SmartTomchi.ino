/*
 * SMART TOMCHI 1.0 - klassik Arduino Nano / ATmega328P
 * Tuproq namligi tahlili, 16x2 LCD va ixtiyoriy sug'orish.
 * Boshlash: README.md va Config.h.
 * Default: aniq belgilangan DEMO + USB ekran nusxasi.
 * DEMO rejimida rele yoqilmaydi.
 */
#include "Config.h"
#include "LogicChecks.h"
#include "Hardware.h"
#include "Calibration.h"
#include "Sensors.h"
#include "Controller.h"
#include "Screen.h"
#include "Console.h"

Calibration calibration;
Sensors sensors;
Controller controller;
Screen screen;
Console console;
bool pinsValid = false;

void setup() {
  Serial.begin(cfg::serialBaud);
  pinsValid = validatePins();
  const bool hardwareReady = pinsValid && cfg::pinsConfirmed;
  Serial.println(F("\n=== SMART TOMCHI 1.0 ==="));
  Serial.println(cfg::demo ? F("DEMO: barcha o'lchovlar namoyish uchun.") :
                            F("REAL: datchik va kalibrlash talab qilinadi."));
  if (!pinsValid) Serial.println(F("XATO: pin takrorlangan yoki Nano uchun yaroqsiz."));
  if (!cfg::pinsConfirmed) Serial.println(F("Pinlar tasdiqlanmagan: tashqi pinlar faollashtirilmadi."));
  controller.begin(hardwareReady, millis());
  if (hardwareReady) calibration.begin();
  sensors.begin(hardwareReady);
  screen.begin(hardwareReady);
  console.help();
}

void loop() {
  uint32_t now = millis();
  controller.tick(sensors.value, calibration, now);
  sensors.tick(now, calibration);
  now = millis();
  controller.tick(sensors.value, calibration, now);
  console.tick(sensors, calibration, controller, now);
  screen.tick(millis(), sensors.value, calibration, controller, pinsValid);
}
