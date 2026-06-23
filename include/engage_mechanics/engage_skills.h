#ifndef GUARD_ENGAGE_MECHANICS_ENGAGE_SKILLS_H
#define GUARD_ENGAGE_MECHANICS_ENGAGE_SKILLS_H

#include "gba/types.h"
#include "engage_mechanics/engage_data.h"
#include "engage_mechanics/engage_skill_registry.h"
#include "engage_mechanics/engage_bond_unlocks.h"

/* Max sync skill slots any single emblem can hold. Drives struct
 * EmblemSyncSkills.skills[] size. Marth has 6 in real data; the rest have 5.
 * Bumping this grows every emblem's reserved storage even if unused slots
 * stay zero-init, so pick the highest known count plus a little headroom. */
enum { SKILL_DEF_COUNT_SYNC_MAX = 6, SKILL_DEF_COUNT_ENGAGE = 1 };

enum SkillEffectKind
{
    SKILL_EFFECT_NONE                = 0,
    SKILL_EFFECT_HP_PCT              = 1, // +X% max HP (X = value)
    SKILL_EFFECT_BATTLE_ATK          = 2, // +N atk during battle
    SKILL_EFFECT_BATTLE_HIT          = 3, // +N hit during battle
    SKILL_EFFECT_BATTLE_AVO          = 4, // +N avoid during battle
    SKILL_EFFECT_BATTLE_CRIT         = 5, // +N crit during battle
    SKILL_EFFECT_BREAK               = 6, // stub: no-op until #16 wires it up
    SKILL_EFFECT_DUAL_STRIKE         = 7, // stub: forces a follow-up
    SKILL_EFFECT_PERCEPTIVE          = 8, // +hit/avo when adjacent to ally
    SKILL_EFFECT_PERCEPTIVE_PLUS     = 9, // upgraded form (Marth Lv 16)
    SKILL_EFFECT_BREAK_DEFENSES      = 10, // +dmg vs defending (was generic BREAK)
    SKILL_EFFECT_UNYIELDING          = 11, // +dmg when low HP
    SKILL_EFFECT_UNYIELDING_PLUS     = 12, // upgraded form (Marth Lv 12)
    SKILL_EFFECT_UNYIELDING_PLUS_PLUS = 13, // upgraded form (Marth Lv 18)
    SKILL_EFFECT_SWORD_AGILITY       = 14, // +spd with sword; .value = tier 1..5
    SKILL_EFFECT_AVOID_BONUS         = 15, // +avoid; .value = 10/15/20/25/30
};

/* Real structs on GBA + Linux host; minimal mirror on macOS where bmunit.h
 * is not host-linkable (variables.h uses Mach-O-incompatible section attrs).
 * Mirrors the established pattern in engage_ring.h.
 *
 * The macOS mirror below matches the **Linux host** layout of struct Unit and
 * struct BattleUnit (compiled with 8-byte pointers on x86_64/arm64) so that
 * host tests behave identically on macOS and Linux. The fields the resolver
 * touches are:
 *   - Unit.uEngageSkillUsed
 *   - Unit.ringEmblemId
 *   - Unit.ringBondLevel
 *   - BattleUnit.battleAttack / battleHitRate / battleAvoidRate / battleCritRate
 *
 * Host tests use struct Unit / struct BattleUnit directly from this header
 * (no hand-rolled stubs) so the resolver's view of the struct is exactly the
 * struct the test sets up. If the real struct layout in bmunit.h changes
 * (e.g., a new field is added before uEngageSkillUsed), this mirror must
 * be updated in lockstep or the resolver tests will diverge. */
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
    unsigned char uEngageSkillUsed;  /* #70 */
    unsigned char ringEmblemId;      /* #49 */
    unsigned char ringBondLevel;     /* #49 */
};

struct BattleUnit {
    struct Unit unit;
    unsigned short weapon;
    unsigned short weaponBefore;
    unsigned int weaponAttributes;
    unsigned char weaponType;
    unsigned char weaponSlotIndex;
    unsigned char _pad_6A[0x72 - 0x60 - 10]; /* 8 bytes: canCounter..pad */
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

#endif /* __APPLE__ */

/* Sword weapon type; bmitem.h is not host-safe so we define it locally.
 * Value matches ITYPE_SWORD in the GBA build (0). */
#ifndef ENGAGE_ITYPE_SWORD
#define ENGAGE_ITYPE_SWORD 0
#endif

extern u8 gEngageBreakDefenses;
extern bool8 (*Engage_AdjacentAllyCheck)(struct Unit *unit);

void ApplySyncSkillsToBattleUnit(struct BattleUnit *bu, struct Unit *unit);
void ApplyEngageSkillToBattleUnit(struct BattleUnit *bu, struct Unit *unit);
bool8 HasInheritedSkill(struct Unit *unit, u8 skillId);

#endif // GUARD_ENGAGE_MECHANICS_ENGAGE_SKILLS_H