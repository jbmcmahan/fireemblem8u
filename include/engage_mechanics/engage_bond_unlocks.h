#ifndef GUARD_ENGAGE_MECHANICS_ENGAGE_BOND_UNLOCKS_H
#define GUARD_ENGAGE_MECHANICS_ENGAGE_BOND_UNLOCKS_H

#include "gba/types.h"

/* Per-event unlock tables. Each row is one unlock: an emblem hits a
 * bond tier and gains the listed skill/item/flag. Same shape, three
 * tables for the three skill flavors; dedicated shapes for weapons
 * and class-change flags. */

/* Sync, inherit, and engage skill unlocks — 3 bytes each:
 * emblemId (0..11), bondLevel (1..20), skillId (index into gSkillDefs). */
struct SkillUnlock
{
    u8 emblemId;
    u8 bondLevel;
    u8 skillId;
};

/* Weapon unlocks — 3 bytes each: emblemId, bondLevel, itemId. */
struct WeaponUnlock
{
    u8 emblemId;
    u8 bondLevel;
    u8 itemId;
};

/* Class-change flag — 2 bytes each: emblemId, bondLevel. */
struct ClassChangeUnlock
{
    u8 emblemId;
    u8 bondLevel;
};

extern struct SkillUnlock gSyncSkillUnlocks[];
extern const u8 gSyncSkillUnlockCount;

extern struct SkillUnlock gInheritSkillUnlocks[];
extern const u8 gInheritSkillUnlockCount;

extern struct SkillUnlock gEngageSkillUnlocks[];
extern const u8 gEngageSkillUnlockCount;

extern struct WeaponUnlock gWeaponUnlocks[];
extern const u8 gWeaponUnlockCount;

extern struct ClassChangeUnlock gClassChangeUnlocks[];
extern const u8 gClassChangeUnlockCount;

#endif // GUARD_ENGAGE_MECHANICS_ENGAGE_BOND_UNLOCKS_H