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

/* Real structs on GBA + Linux host; minimal mirror on macOS where bmunit.h
 * is not host-linkable (variables.h uses Mach-O-incompatible section attrs).
 * Mirrors the established pattern in engage_ring.h. */
#if !defined(__APPLE__)
#include "bmbattle.h"
#else
struct Unit {
    unsigned char _pad_to_maxHP[0x12];
    signed char   maxHP;                    /* 0x12 */
    unsigned char _pad_to_used[0x47 - 0x13];
    unsigned char uEngageSkillUsed;         /* 0x47 (#70) */
    unsigned char ringEmblemId;             /* 0x48 (#49) */
    unsigned char ringBondLevel;            /* 0x49 (#49) */
};
struct BattleUnit {
    struct Unit unit;                       /* 0x00 */
    unsigned char _pad_to_atk[0x5A - sizeof(struct Unit)];
    short battleAttack;                     /* 0x5A */
    unsigned char _pad_to_hit[0x60 - 0x5C];
    short battleHitRate;                    /* 0x60 */
    short battleAvoidRate;                  /* 0x62 */
    unsigned char _pad_to_crit[0x66 - 0x64];
    short battleCritRate;                   /* 0x66 */
};
#endif

void ApplySyncSkillsToBattleUnit(struct BattleUnit *bu, struct Unit *unit);
void ApplyEngageSkillToBattleUnit(struct BattleUnit *bu, struct Unit *unit);
bool8 HasInheritedSkill(struct Unit *unit, u8 skillId);

#endif // GUARD_ENGAGE_MECHANICS_ENGAGE_SKILLS_H