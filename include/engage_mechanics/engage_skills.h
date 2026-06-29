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
 * Mirrors the established pattern in engage_ring.h.
 *
 * The macOS mirror below matches the **Linux host** layout of struct Unit and
 * struct BattleUnit (compiled with 8-byte pointers on x86_64/arm64) so that
 * host tests behave identically on macOS and Linux. The fields the resolver
 * touches are:
 *   - UNIT_RING_EMBLEM_ID
 *   - UNIT_RING_BOND_LEVEL
 *   - UNIT_ENGAGE_SKILL_USED
 *   - BattleUnit.battleAttack / battleHitRate / battleAvoidRate / battleCritRate
 *
 * Host tests use struct Unit / struct BattleUnit directly from this header
 * (no hand-rolled stubs) so the resolver's view of the struct is exactly the
 * struct the test sets up. If the real struct layout in bmunit.h changes,
 * this mirror must be updated in lockstep or the resolver tests will
 * diverge. */
#if !defined(__APPLE__)
#include "bmbattle.h"
#else
struct Unit {
    void * pCharacterData;
    void * pClassData;
    signed char level;
    unsigned char exp;
    unsigned char aiFlags;
    signed char index;
    unsigned int state;
    signed char xPos;
    signed char yPos;
    signed char maxHP;
    signed char curHP;
    signed char pow;
    signed char skl;
    signed char spd;
    signed char def;
    signed char res;
    signed char lck;
    signed char conBonus;
    unsigned char rescue;
    unsigned char ballistaIndex;
    signed char movBonus;
    unsigned short items[5];
    unsigned char ranks[8];
    unsigned char statusIndex    : 4;
    unsigned char statusDuration : 4;
    unsigned char torchDuration  : 4;
    unsigned char barrierDuration : 4;
    unsigned char supports[7];
    signed char supportBits;
    unsigned char _u3A;
    unsigned char _u3B;
    void * pMapSpriteHandle;
    unsigned short ai_config;
    unsigned char ai1;
    unsigned char ai_a_pc;
    unsigned char ai2;
    unsigned char ai_b_pc;
    unsigned char ai_counter;
};

struct BattleUnit {
    struct Unit unit;
    unsigned char _pad_60[0x72 - 0x60];
    short battleAttack;
    short battleDefense;
    short battleSpeed;
    short battleHitRate;
    short battleAvoidRate;
    short battleEffectiveHitRate;
    short battleCritRate;
    short battleDodgeRate;
    short battleEffectiveCritRate;
    /* 84 */ short battleSilencerRate;
}; /* sizeof = 0x86 */
#endif

#ifndef UNIT_RING_EMBLEM_NONE
#define UNIT_RING_EMBLEM_NONE 0xFF
#define UNIT_RING_BOND_LEVEL_MASK 0x7F
#define UNIT_ENGAGE_SKILL_USED_FLAG 0x80

#define UNIT_RING_EMBLEM_ID(unit) ((unit)->_u3A)
#define UNIT_RING_BOND_LEVEL(unit) ((unit)->_u3B & UNIT_RING_BOND_LEVEL_MASK)
#define UNIT_ENGAGE_SKILL_USED(unit) (((unit)->_u3B & UNIT_ENGAGE_SKILL_USED_FLAG) != 0)

#define UNIT_SET_RING_EMBLEM_ID(unit, value) ((unit)->_u3A = (value))
#define UNIT_SET_RING_BOND_LEVEL(unit, value) \
    ((unit)->_u3B = ((unit)->_u3B & UNIT_ENGAGE_SKILL_USED_FLAG) | ((value) & UNIT_RING_BOND_LEVEL_MASK))
#define UNIT_SET_ENGAGE_SKILL_USED(unit, value) \
    ((unit)->_u3B = ((unit)->_u3B & UNIT_RING_BOND_LEVEL_MASK) | ((value) ? UNIT_ENGAGE_SKILL_USED_FLAG : 0))
#define UNIT_CLEAR_ENGAGE_STATE(unit) \
    do { \
        UNIT_SET_RING_EMBLEM_ID((unit), UNIT_RING_EMBLEM_NONE); \
        (unit)->_u3B = 0; \
    } while (0)
#endif

void ApplySyncSkillsToBattleUnit(struct BattleUnit *bu, struct Unit *unit);
void ApplyEngageSkillToBattleUnit(struct BattleUnit *bu, struct Unit *unit);
bool8 HasInheritedSkill(struct Unit *unit, u8 skillId);

#endif // GUARD_ENGAGE_MECHANICS_ENGAGE_SKILLS_H
