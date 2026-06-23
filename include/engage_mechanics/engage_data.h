#ifndef GUARD_ENGAGE_MECHANICS_ENGAGE_DATA_H
#define GUARD_ENGAGE_MECHANICS_ENGAGE_DATA_H

#include "gba/types.h"

struct ProcCmd; // forward-declared so this header stays host-portable

enum { EMBLEM_DEF_COUNT = 12 };

struct EmblemDef
{
    /* Display name. NUL-terminated ASCII, owned by the table. */
    const char * name;

    /* 1-byte item ID of the Emblem's engage weapon. */
    u8 engageWeaponItemId;

    /* ID of the engage skill. Zero = unassigned. */
    u8 engageSkill;

    /* Bond growth per stat. Index 0..6 = HP/STR/MAG/SKL/SPD/DEF/RES. */
    s8 bondGrowthStats[7];

    /* Proc to invoke on chain attack. NULL = none. */
    struct ProcCmd * chainAttackProc;

    /* Weapon triangle (sword/lance/axe/...). Zero = neutral. */
    u8 weaponTriangle;
};

/* No CONST_DATA on this extern — section attr lives on the definition in
 * engage_data.c (PR #30). Mirroring variables.h's `extern CONST_DATA` would
 * pull in prelude.h's `__attribute__((section(".data")))` and break the
 * Mach-O host test build (GCC rejects section attrs on Mach-O externs). */
extern struct EmblemDef gEmblemDefs[EMBLEM_DEF_COUNT];

#endif // GUARD_ENGAGE_MECHANICS_ENGAGE_DATA_H
