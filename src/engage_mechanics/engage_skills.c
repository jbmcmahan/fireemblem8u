#include "engage_mechanics/engage_skills.h"
#include "engage_mechanics/engage_skill_registry.h"
#include "engage_mechanics/engage_bond_unlocks.h"
#include "engage_mechanics/engage_synced_bonus.h"
#include <stddef.h>

#define NO_RING 0xFF

u8 gEngageBreakDefenses = 0;
bool8 (*Engage_AdjacentAllyCheck)(struct Unit *unit) = NULL;

/* Walk one skill-unlock table and update family tracking variables.
 * Generic BATTLE_* effects are applied immediately to bu; family skills
 * (PERCEPTIVE, UNYIELDING, SWORD_AGILITY, AVOID_BONUS, BREAK_DEFENSES)
 * update the tracking state so the caller can apply highest-tier winners. */
static void walkTable(
    struct BattleUnit *bu,
    u8 emblemId, u8 bondLevel,
    const struct SkillUnlock *table, u8 count,
    u8 *bestPerceptive, s8 *bestPerceptiveVal,
    u8 *bestUnyielding, s8 *bestUnyieldingVal,
    s8 *bestSwordAgiVal,
    s8 *avoidBonusVal,
    u8 *breakDefenses)
{
    for (u8 i = 0; i < count; ++i) {
        struct SkillUnlock u = table[i];
        if (u.emblemId != emblemId) continue;
        if (u.bondLevel > bondLevel) continue;
        struct SkillDef sk = gSkillDefs[u.skillId];
        switch (sk.kind) {
        case SKILL_EFFECT_BATTLE_ATK:  bu->battleAttack    += sk.value; break;
        case SKILL_EFFECT_BATTLE_HIT:  bu->battleHitRate   += sk.value; break;
        case SKILL_EFFECT_BATTLE_AVO:  bu->battleAvoidRate += sk.value; break;
        case SKILL_EFFECT_BATTLE_CRIT: bu->battleCritRate  += sk.value; break;
        case SKILL_EFFECT_HP_PCT:      break;
        case SKILL_EFFECT_BREAK:       break;
        case SKILL_EFFECT_DUAL_STRIKE: break;
        case SKILL_EFFECT_PERCEPTIVE:
            if (*bestPerceptive < 1) { *bestPerceptive = 1; *bestPerceptiveVal = sk.value; }
            break;
        case SKILL_EFFECT_PERCEPTIVE_PLUS:
            if (*bestPerceptive < 2) { *bestPerceptive = 2; *bestPerceptiveVal = sk.value; }
            break;
        case SKILL_EFFECT_UNYIELDING:
            if (*bestUnyielding < 1) { *bestUnyielding = 1; *bestUnyieldingVal = sk.value; }
            break;
        case SKILL_EFFECT_UNYIELDING_PLUS:
            if (*bestUnyielding < 2) { *bestUnyielding = 2; *bestUnyieldingVal = sk.value; }
            break;
        case SKILL_EFFECT_UNYIELDING_PLUS_PLUS:
            if (*bestUnyielding < 3) { *bestUnyielding = 3; *bestUnyieldingVal = sk.value; }
            break;
        case SKILL_EFFECT_SWORD_AGILITY:
            if (sk.value > *bestSwordAgiVal) *bestSwordAgiVal = sk.value;
            break;
        case SKILL_EFFECT_AVOID_BONUS:
            if (sk.value > *avoidBonusVal) *avoidBonusVal = sk.value;
            break;
        case SKILL_EFFECT_BREAK_DEFENSES:
            *breakDefenses = 1;
            break;
        default: break;
        }
    }
}

/* Apply synced and inherited skills for the unit's equipped ring.
 *
 * Two-pass design:
 *   Pass 1 — walks gSyncSkillUnlocks and gInheritSkillUnlocks, filtering by
 *             emblemId and bondLevel. Generic BATTLE_* effects apply immediately.
 *             Family skills (Perceptive, Unyielding, Sword Agility, Avoid Bonus,
 *             Break Defenses) are tracked so upgraded forms suppress base forms.
 *   Pass 2 — applies each family's highest-tier winner with its own condition:
 *             Unyielding:  HP <= 50 % of max
 *             Perceptive:  adjacent ally (via injectable Engage_AdjacentAllyCheck)
 *             Sword Agility: weapon is a sword (weaponType == ENGAGE_ITYPE_SWORD)
 *             Avoid Bonus:  unconditional
 *             Break Defenses: sets gEngageBreakDefenses flag (damage hook deferred) */
void ApplySyncSkillsToBattleUnit(struct BattleUnit *bu, struct Unit *unit)
{
    gEngageBreakDefenses = 0;

    u8 emblemId = unit->ringEmblemId;
    if (emblemId == NO_RING) return;

    u8 bondLevel = unit->ringBondLevel;
    if (bondLevel == 0) return;

    /* Cumulative bonus at this bond level — read but not yet applied.
     * Fields will be wired when struct BattleUnit gains battleMagic etc. */
    struct BonusRow b = gSyncedBonuses[emblemId][bondLevel - 1];
    (void)b;

    /* Family-tracking state for Pass 1. */
    u8 bestPerceptive   = 0; /* 0=none, 1=base, 2=plus */
    s8 bestPerceptiveVal = 0;
    u8 bestUnyielding   = 0; /* 0=none, 1=base, 2=plus, 3=plusplus */
    s8 bestUnyieldingVal = 0;
    s8 bestSwordAgiVal  = 0; /* 0=none, or highest tier value (1/2/3) */
    s8 avoidBonusVal    = 0;
    u8 breakDefenses    = 0;

    /* Pass 1: walk both unlock tables. */
    walkTable(bu, emblemId, bondLevel,
              gSyncSkillUnlocks, gSyncSkillUnlockCount,
              &bestPerceptive, &bestPerceptiveVal,
              &bestUnyielding, &bestUnyieldingVal,
              &bestSwordAgiVal, &avoidBonusVal, &breakDefenses);
    walkTable(bu, emblemId, bondLevel,
              gInheritSkillUnlocks, gInheritSkillUnlockCount,
              &bestPerceptive, &bestPerceptiveVal,
              &bestUnyielding, &bestUnyieldingVal,
              &bestSwordAgiVal, &avoidBonusVal, &breakDefenses);

    /* Pass 2: apply family winners. */
    if (bestUnyielding > 0
            && unit->maxHP > 0
            && (unit->curHP * 2) <= unit->maxHP)
        bu->battleAttack += bestUnyieldingVal;

    if (bestPerceptive > 0
            && Engage_AdjacentAllyCheck
            && Engage_AdjacentAllyCheck(unit)) {
        bu->battleHitRate   += bestPerceptiveVal;
        bu->battleAvoidRate += bestPerceptiveVal;
    }

    if (bestSwordAgiVal > 0 && bu->weaponType == ENGAGE_ITYPE_SWORD)
        bu->battleSpeed += bestSwordAgiVal;

    if (avoidBonusVal > 0)
        bu->battleAvoidRate += avoidBonusVal;

    if (breakDefenses) {
        gEngageBreakDefenses = 1;
        /* TODO(#94-followup): damage multiplier hook deferred */
    }
}

/* Apply the engage skill (one per emblem). The engage skill is keyed by
 * emblemId; .bondLevel on the unlock row is always 0 today (#85-A) — the
 * engage skill unlocks at bond level 0 (i.e., is always available when
 * the ring is equipped). uEngageSkillUsed makes the call idempotent. */
void ApplyEngageSkillToBattleUnit(struct BattleUnit *bu, struct Unit *unit)
{
    if (unit->ringEmblemId == NO_RING) return;
    if (unit->uEngageSkillUsed) return;
    {
        struct SkillUnlock u = gEngageSkillUnlocks[unit->ringEmblemId];
        struct SkillDef sk = gSkillDefs[u.skillId];
        unit->uEngageSkillUsed = 1;
        switch (sk.kind) {
        case SKILL_EFFECT_BATTLE_ATK:  bu->battleAttack    += sk.value; break;
        case SKILL_EFFECT_BATTLE_HIT:  bu->battleHitRate   += sk.value; break;
        case SKILL_EFFECT_BATTLE_AVO:  bu->battleAvoidRate += sk.value; break;
        case SKILL_EFFECT_BATTLE_CRIT: bu->battleCritRate  += sk.value; break;
        default: break;
        }
    }
}

bool8 HasInheritedSkill(struct Unit *unit, u8 skillId)
{
    (void)unit; (void)skillId;
    for (u8 i = 0; i < gInheritSkillUnlockCount; ++i) {
        if (gInheritSkillUnlocks[i].skillId == skillId)
            return 1;
    }
    return 0;
}
