#include "engage_mechanics/engage_skills.h"
// Forward declarations for item type checks
#ifndef ITYPE_SWORD
#define ITYPE_SWORD 0
#endif
struct ItemData { u8 weaponType; };
static struct ItemData gItemData[256] = {0};
static int GetItemType(int item) {
    return gItemData[item & 0xFF].weaponType;
}
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
void ApplySyncSkillsToBattleUnit(struct BattleUnit *bu, struct Unit *unit, bool8 isInitiator)
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

        /* First pass: find max unlocked tier per group */
        u8 maxTier[16] = {0};
        for (u8 i = 0; i < gSyncSkillUnlockCount; ++i) {
            struct SkillUnlock u = gSyncSkillUnlocks[i];
            if (u.emblemId != emblemId) continue;
            if (u.bondLevel > bondLevel) continue;
            struct SkillDef sk = gSkillDefs[u.skillId];
            if (sk.group != 0 && u.bondLevel > maxTier[sk.group])
                maxTier[sk.group] = u.bondLevel;
        }

        /* Second pass: apply, skipping suppressed entries */
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
            case SKILL_EFFECT_DIVINE_SPEED: /* deferred to follow-up hook */ break;
            case SKILL_EFFECT_PERCEPTIVE:
            case SKILL_EFFECT_PERCEPTIVE_PLUS:
                if (isInitiator)
                    bu->battleAvoidRate += sk.value;
                break;
            case SKILL_EFFECT_SWORD_AGILITY:
                if (GetItemType(bu->weapon) == ITYPE_SWORD) {
                    bu->battleAvoidRate += sk.value;
                    bu->battleCritRate  -= 10;
                }
                break;
            default: break;
            }
        }
    }
}

/* Apply the engage skill (one per emblem). The engage skill is keyed by
 * emblemId; .bondLevel on the unlock row is always 0 today (#85-A) — the
 * engage skill unlocks at bond level 0 (i.e., is always available when
 * the ring is equipped). uEngageSkillUsed makes the call idempotent. */
void ApplyEngageSkillToBattleUnit(struct BattleUnit *bu, struct Unit *unit, bool8 isInitiator)
{
    if (unit->ringEmblemId == NO_RING) return;
    if (unit->uEngageSkillUsed) return;
    {
        struct SkillUnlock u = gEngageSkillUnlocks[unit->ringEmblemId];
        struct SkillDef sk = gSkillDefs[u.skillId];
        switch (sk.kind) {
        case SKILL_EFFECT_BATTLE_ATK:  bu->battleAttack    += sk.value; unit->uEngageSkillUsed = 1; break;
        case SKILL_EFFECT_BATTLE_HIT:  bu->battleHitRate   += sk.value; unit->uEngageSkillUsed = 1; break;
        case SKILL_EFFECT_BATTLE_AVO:  bu->battleAvoidRate += sk.value; unit->uEngageSkillUsed = 1; break;
        case SKILL_EFFECT_BATTLE_CRIT: bu->battleCritRate  += sk.value; unit->uEngageSkillUsed = 1; break;
        case SKILL_EFFECT_DIVINE_SPEED: /* deferred to follow-up hook */ break;
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

bool8 Engage_ShouldForceDivineSpeed(struct Unit *unit)
{
    if (unit->ringEmblemId == NO_RING) return 0;
    if (unit->uEngageSkillUsed) return 0;
    struct SkillUnlock u = gEngageSkillUnlocks[unit->ringEmblemId];
    struct SkillDef sk = gSkillDefs[u.skillId];
    return (sk.kind == SKILL_EFFECT_DIVINE_SPEED);
}