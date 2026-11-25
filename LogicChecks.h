#pragma once
#include "Logic.h"

// Compile vaqtida ham sinovdan o'tadi; Nano RAM/xotirasini band qilmaydi.
static_assert(logic::moisture(800, 800, 350) == 0, "dry endpoint");
static_assert(logic::moisture(350, 800, 350) == 100, "wet endpoint");
static_assert(logic::moisture(575, 800, 350) == 50, "midpoint");
static_assert(logic::moisture(200, 200, 800) == 0, "reverse dry");
static_assert(logic::moisture(800, 200, 800) == 100, "reverse wet");
static_assert(logic::moisture(50, 800, 350) == 100, "clamp wet");
static_assert(logic::moisture(1000, 800, 350) == 0, "clamp dry");
static_assert(logic::moisture(500, 500, 500) == -1, "zero span");
static_assert(!logic::calibrationValid(500, 550), "small span");
static_assert(!logic::calibrationValid(0, 700), "rail endpoint");
static_assert(logic::elapsed(10, UINT32_MAX - 9, 20), "millis rollover");
static_assert(!logic::elapsed(9, UINT32_MAX - 9, 20), "rollover before due");
static_assert(logic::shouldStart(true, false, false, true, 30, 30), "start");
static_assert(!logic::shouldStart(false, false, false, true, 20, 30), "interlock");
static_assert(!logic::shouldStart(true, false, true, true, 20, 30), "fault");
static_assert(!logic::shouldStart(true, false, false, false, 20, 30), "rest");
static_assert(!logic::shouldStart(true, false, false, true, -1, 30), "invalid");
static_assert(logic::shouldStop(false, true, 20, 55), "sensor failure");
static_assert(logic::shouldStop(true, true, 55, 55), "wet stop");
static_assert(!logic::shouldStop(true, true, 40, 55), "hysteresis");
