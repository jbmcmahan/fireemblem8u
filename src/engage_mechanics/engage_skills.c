#include "engage_mechanics/engage_skills.h"
#include "engage_mechanics/engage_skill_registry.h"
#include "engage_mechanics/engage_bond_unlocks.h"
#include "engage_mechanics/engage_synced_bonus.h"

#define NO_RING 0xFF

/* Apply synced skills for (emblemId, bondLevel). Walks
 * gSyncSkillUnlocks[] for matching rows (emblemId match AND
 * bondLevel <= unit's bond level) and applies each skill effect.
 *
 * The cumulative bonus table (gSyncedBonuses) is read here but its
 * fields can't all be applied yet: struct BattleUnit has no
 * battleMaxHp / battleSkill / battleResistance, and struct Unit has
 * no .mag. The fields that do exist (battleAttack, battleDefense,
 * battleSpeed) would be written but gSyncedBonuses is zero-init in
 * #85-A so the writes are no-ops today. When struct Unit.mag lands
 * and struct BattleUnit.battleMagic lands in a future issue, this
 * resolver just adds those writes on one line. See PLAN.md Open Q1.
 *
 * HP_PCT and other deferred kinds still no-op as before (#78/#81). */
void ApplySyncSkillsToBattleUnit(struct BattleUnit *bu, struct Unit *unit)
{
    u8 emblemId = unit->ringEmblemId;
    if (emblemId == NO_RING) return;
    {
        u8 bondLevel = unit->ringBondLevel;
        if (bondLevel == 0) return;

        /* Cumulative bonus at this bond level. Read but currently
         * zero-init in #85-A; the field-set will be wired as the
         * struct grows. */
        struct BonusRow b = gSyncedBonuses[emblemId][bondLevel - 1];
        (void)b;

        /* Walk sync skill unlocks. */
        for (u8 i = 0; i < gSyncSkillUnlockCount; ++i) {
            struct SkillUnlock u = gSyncSkillUnlocks[i];
            if (u.emblemId != emblemId) continue;
            if (u.bondLevel > bondLevel) continue;
            struct SkillDef sk = gSkillDefs[u.skillId];
            switch (sk.kind) {
            case SKILL_EFFECT_BATTLE_ATK:  bu->battleAttack    += sk.value; break;
            case SKILL_EFFECT_BATTLE_HIT:  bu->battleHitRate   += sk.value; break;
            case SKILL_EFFECT_BATTLE_AVO:  bu->battleAvoidRate += sk.value; break;
            case SKILL_EFFECT_BATTLE_CRIT: bu->battleCritRate  += sk.value; break;
            case SKILL_EFFECT_HP_PCT:      /* deferred: no battleMaxHp field */ break;
            case SKILL_EFFECT_BREAK:       /* stub: wired in #16 */ break;
            case SKILL_EFFECT_DUAL_STRIKE: /* stub: chain-attack issue */ break;
            default: break;
            }
        }
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