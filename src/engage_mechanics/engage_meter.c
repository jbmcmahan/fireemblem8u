#include "engage_mechanics/engage_api.h"

#include "gba/types.h"

/* Faction masks from bmunit.h — duplicated here to keep this TU host-pure. */
#define METER_FACTION_BLUE  0x00
#define METER_FACTION_GREEN 0x40
#define METER_FACTION_RED   0x80

u8 EngageMeter_Clamp(u16 value)

{
    if (value > ENGAGE_METER_MAX)
        return ENGAGE_METER_MAX;
    return (u8)value;
}

u8 EngageMeter_Add(u8 current, u8 amount)
{
    return EngageMeter_Clamp((u16)current + (u16)amount);
}

bool8 EngageMeter_IsFull(u8 value)
{
    return value >= ENGAGE_METER_MAX;
}

/* Pure helper: compute IWRAM table slot for a unit index.
   Returns -1 for invalid indices (purple faction, out of range, zero index). */
s8 EngageMeter_SlotForUnit(u8 index)
{
    u8 faction;
    s8 slot;

    if (index == 0) return -1;
    faction = index & 0xC0;
    slot = (index & 0x3F) - 1; // zero-based slot within faction table
    switch (faction) {
        case METER_FACTION_BLUE:
            if (slot >= 62) return -1;
            break;
        case METER_FACTION_GREEN:
            if (slot >= 20) return -1;
            break;
        case METER_FACTION_RED:
            if (slot >= 50) return -1;
            break;
        default:
            return -1; // purple or unknown
    }
    return slot;
}
