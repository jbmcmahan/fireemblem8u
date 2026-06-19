#ifndef GUARD_ENGAGE_MECHANICS_ENGAGE_METER_H
#define GUARD_ENGAGE_MECHANICS_ENGAGE_METER_H

#include "gba/types.h"

#define ENGAGE_METER_MAX 100

// Saturating add of `amount` to `current`, clamped to ENGAGE_METER_MAX.
u8 EngageMeter_Add(u8 current, u8 amount);
// Clamp an arbitrary value into [0, ENGAGE_METER_MAX].
u8 EngageMeter_Clamp(u16 value);
// TRUE iff value has reached the cap.
bool8 EngageMeter_IsFull(u8 value);

#endif // GUARD_ENGAGE_MECHANICS_ENGAGE_METER_H