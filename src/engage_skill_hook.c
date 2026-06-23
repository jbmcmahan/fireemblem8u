#include "global.h"
#include "bmbattle.h"
#include "bmunit.h"
#include "bmitem.h"
#include "constants/items.h"
#include "engage_mechanics/engage_skills.h"

#define NO_RING 0xFF

// Thin hook: original stats first, then add the attacker's unlocked
// sync-skill bonuses. We deliberately do NOT apply to the defender here
// — BattleGenerate calls ComputeBattleUnitStats twice with the actor
// and target swapped (src/bmbattle.c:186-187), so each unit gets its
// own bonuses applied exactly once as the "attacker" of one of the two
// calls. Applying to both parameters here would double-count the bonus
// for the unit that is attacker in both calls.
void Engage_ComputeBattleUnitStats(struct BattleUnit* attacker, struct BattleUnit* defender)
{
    ComputeBattleUnitStats(attacker, defender);
    if (attacker->unit) {
        ApplySyncSkillsToBattleUnit(attacker, &attacker->unit);
    }
}

static bool8 sEngageForcedFollowUp = FALSE;

s8 Engage_BattleGetFollowUpOrder(struct BattleUnit** outAttacker, struct BattleUnit** outDefender) {
    sEngageForcedFollowUp = FALSE;

    /* 1. Call original first. If natural follow-up exists, return it. */
    if (BattleGetFollowUpOrder(outAttacker, outDefender))
        return TRUE;

    /* 2. No natural follow-up. Check DUAL_STRIKE. */
    struct Unit* unit = &gBattleActor.unit;
    if (unit->ringEmblemId == NO_RING) return FALSE;
    if (unit->uEngageSkillUsed) return FALSE;
    /* TODO(#12): replace with IsEngaged(unit) check */

    struct SkillUnlock u = gEngageSkillUnlocks[unit->ringEmblemId];
    struct SkillDef sk = gSkillDefs[u.skillId];
    if (sk.kind != SKILL_EFFECT_DUAL_STRIKE) return FALSE;

    /* 3. Force follow-up with gBattleActor as attacker. */
    *outAttacker = &gBattleActor;
    *outDefender = &gBattleTarget;

    /* 4. Respect the same weapon-effect guards as the original. */
    if (GetItemWeaponEffect((*outAttacker)->weaponBefore) == WPN_EFFECT_HPHALVE)
        return FALSE;
    if (GetItemIndex((*outAttacker)->weapon) == ITEM_MONSTER_STONE)
        return FALSE;

    sEngageForcedFollowUp = TRUE;
    return TRUE;
}

void Engage_ConsumeDualStrike(void) {
    if (sEngageForcedFollowUp) {
        /* Don't consume the skill in simulation mode (AI forecasts, battle preview). */
        if (!(gBattleStats.config & BATTLE_CONFIG_SIMULATE))
            gBattleActor.unit.uEngageSkillUsed = 1;
        sEngageForcedFollowUp = FALSE;
    }
}