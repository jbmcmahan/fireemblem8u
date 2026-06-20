#ifndef GUARD_ENGAGE_MECHANICS_ENGAGE_SKILLS_H
#define GUARD_ENGAGE_MECHANICS_ENGAGE_SKILLS_H

#include "gba/types.h"

enum { SKILL_DEF_COUNT_SYNC = 5, SKILL_DEF_COUNT_ENGAGE = 1 };

enum SkillEffectKind
{
    SKILL_EFFECT_NONE         = 0,
    SKILL_EFFECT_HP_PCT       = 1, // +X% max HP (X = value)
    SKILL_EFFECT_BATTLE_ATK   = 2, // +N atk during battle
    SKILL_EFFECT_BATTLE_HIT   = 3, // +N hit during battle
    SKILL_EFFECT_BATTLE_AVO   = 4, // +N avoid during battle
    SKILL_EFFECT_BATTLE_CRIT  = 5, // +N crit during battle
    SKILL_EFFECT_BREAK        = 6, // stub: no-op until #16 wires it up
    SKILL_EFFECT_DUAL_STRIKE  = 7, // stub: forces a follow-up
};

struct SkillDef
{
    u8  kind;       // SkillEffectKind
    s8  value;      // signed stat delta (HP_PCT stores X here)
    u8  emblemId;   // owning Emblem (0..11)
    u8  tier;       // bond tier that unlocks this skill (1/3/5/9/15)
    u8  _pad[4];    // reserved for future fields; pin size
};

extern struct SkillDef gSyncSkillDefs[12 * SKILL_DEF_COUNT_SYNC];
extern struct SkillDef gEngageSkillDefs[12 * SKILL_DEF_COUNT_ENGAGE];

#endif // GUARD_ENGAGE_MECHANICS_ENGAGE_SKILLS_H
