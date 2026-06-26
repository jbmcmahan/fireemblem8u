// Engage meter IWRAM tables and unit-aware wrappers (GBA-only)
#include "global.h"
#include "gba/defines.h"
#include "bmunit.h"
#include "hardware.h"
#include "engage_mechanics/engage_api.h"

IWRAM_DATA u8 gEngageMeterBlue[62];
IWRAM_DATA u8 gEngageMeterRed[50];
IWRAM_DATA u8 gEngageMeterGreen[20];

u8 GetEngageMeter(struct Unit* unit)
{
    s8 slot;

    if (!unit) return 0;
    slot = EngageMeter_SlotForUnit(unit->index);
    if (slot < 0) return 0;
    switch (unit->index & 0xC0) {
        case 0x00: return gEngageMeterBlue[slot];
        case 0x40: return gEngageMeterGreen[slot];
        case 0x80: return gEngageMeterRed[slot];
        default:   return 0;
    }
}

u8 AddEngageMeter(struct Unit* unit, u8 amount)
{
    u8 cur = GetEngageMeter(unit);
    u8 newv = EngageMeter_Add(cur, amount);
    SetEngageMeter(unit, newv);
    return newv;
}

void SetEngageMeter(struct Unit* unit, u8 value)
{
    s8 slot;

    if (!unit) return;
    slot = EngageMeter_SlotForUnit(unit->index);
    if (slot < 0) return;
    value = EngageMeter_Clamp(value);
    switch (unit->index & 0xC0) {
        case 0x00: gEngageMeterBlue[slot] = value; break;
        case 0x40: gEngageMeterGreen[slot] = value; break;
        case 0x80: gEngageMeterRed[slot] = value; break;
        default: break;
    }
}

void ResetEngageMeter(struct Unit* unit)
{
    SetEngageMeter(unit, 0);
}

void ResetAllEngageMeters(void)
{
    CpuFill16(0, gEngageMeterBlue, sizeof(gEngageMeterBlue));
    CpuFill16(0, gEngageMeterRed,  sizeof(gEngageMeterRed));
    CpuFill16(0, gEngageMeterGreen, sizeof(gEngageMeterGreen));
}
