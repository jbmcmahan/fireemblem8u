#ifndef GUARD_ENGAGE_MECHANICS_ENGAGE_SYNCED_BONUS_H
#define GUARD_ENGAGE_MECHANICS_ENGAGE_SYNCED_BONUS_H

#include "gba/types.h"
#include "engage_mechanics/engage_data.h"

/* Cumulative stat bonus per (emblem, bondLevel). Engage bond bonuses
 * are monotonic (never decrease) so cumulative encoding is valid and
 * saves space vs. per-level deltas. Packed 4 bits per stat × 7 stats
 * in a single u32 — compiler pads to 4 bytes per row.
 *
 * mag is reserved-but-unused: FE8's struct Unit has no .mag field.
 * When struct Unit.mag lands in a future issue, the resolver just adds
 * bu->battleMagic += b.mag on one line. See PLAN.md Open Q1. */
struct BonusRow
{
    u8 hp  : 4;   // bits  0-3
    u8 str : 4;   // bits  4-7
    u8 mag : 4;   // bits  8-11 (unused until struct Unit.mag lands)
    u8 skl : 4;   // bits 12-15
    u8 spd : 4;   // bits 16-19
    u8 def : 4;   // bits 20-23
    u8 res : 4;   // bits 24-27
};

/* Max stat delta: real Engage max observed is +5; +15 (4-bit max)
 * gives 3x wiggle room for FE8 integration without re-packing. */
extern struct BonusRow gSyncedBonuses[EMBLEM_DEF_COUNT][20];

#endif // GUARD_ENGAGE_MECHANICS_ENGAGE_SYNCED_BONUS_H