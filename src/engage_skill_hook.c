#include "global.h"
#include "bmbattle.h"
#include "bmunit.h"
#include "engage_mechanics/engage_skills.h"

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
        ApplySyncSkillsToBattleUnit(attacker, &attacker->unit, true);
    }
}

// Follow-up hook for Divine Speed
s8 Engage_GetFollowUpOrder(struct BattleUnit** outAttacker, struct BattleUnit** outDefender)
{
    s8 result = BattleGetFollowUpOrder(outAttacker, outDefender);
    if (result) return TRUE; // vanilla follow-up

    if (gBattleStats.config & BATTLE_CONFIG_SIMULATE) return FALSE;

    if (Engage_ShouldForceDivineSpeed(&gBattleActor.unit))
    {
        *outAttacker = &gBattleActor;
        *outDefender = &gBattleTarget;
        gBattleHitIterator->attributes |= BATTLE_HIT_ATTR_HALFDMG;
        gBattleActor.unit.uEngageSkillUsed = 1;
        return TRUE;
    }

    return FALSE;
}