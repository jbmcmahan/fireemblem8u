#include "engage_mechanics/engage_meter.h"

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
