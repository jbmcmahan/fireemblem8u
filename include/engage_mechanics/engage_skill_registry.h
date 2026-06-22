#ifndef GUARD_ENGAGE_MECHANICS_ENGAGE_SKILL_REGISTRY_H
#define GUARD_ENGAGE_MECHANICS_ENGAGE_SKILL_REGISTRY_H

#include "gba/types.h"

/* Master registry of every named skill in the game. Each skill has a
 * stable ID; unlock tables reference skills by ID. This lets multiple
 * emblems unlock the same skill (e.g., Perceptive, Perceptive+) without
 * duplicating per-emblem copies.
 *
 * The slim shape (kind + value, no emblemId, no tier) lives here. The
 * emblem and bond tier that unlock a skill live on the unlock-row
 * structs in engage_bond_unlocks.h. */
struct SkillDef
{
    u8 kind;     // SkillEffectKind
    s8 value;    // signed stat delta (HP_PCT stores X here)
};

/* Index 0 reserved as the SKILL_EFFECT_NONE sentinel. Grow as needed. */
enum { SKILL_DEF_COUNT = 64 };

extern struct SkillDef gSkillDefs[SKILL_DEF_COUNT];

#endif // GUARD_ENGAGE_MECHANICS_ENGAGE_SKILL_REGISTRY_H