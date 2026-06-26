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

/* Forward declaration for unit-aware wrappers (GBA-only) */
struct Unit;

/* Pure helper: returns meter slot for a unit index, -1 if invalid */
s8 EngageMeter_SlotForUnit(u8 index);

/* Unit-aware wrappers (implemented in engage_meter_hook.c) */
/* Returns current engage meter value for a unit */
u8 GetEngageMeter(struct Unit* unit);
/* Adds amount to unit's engage meter, returns new value */
u8 AddEngageMeter(struct Unit* unit, u8 amount);
/* Sets unit's engage meter to value (clamped) */
void SetEngageMeter(struct Unit* unit, u8 value);
/* Resets a single unit's engage meter to 0 */
void ResetEngageMeter(struct Unit* unit);
/* Resets all engage meters (per chapter) */
void ResetAllEngageMeters(void);

#endif // GUARD_ENGAGE_MECHANICS_ENGAGE_METER_H
