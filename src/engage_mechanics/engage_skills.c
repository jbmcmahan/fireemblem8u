#include "engage_mechanics/engage_skills.h"

void ApplySyncSkillsToBattleUnit(struct BattleUnit *bu, struct Unit *unit)
{
    u8 emblemId = UNIT_RING_EMBLEM_ID(unit);
    u8 bondLevel;
    u8 slot;

    if (emblemId == UNIT_RING_EMBLEM_NONE) return;
    if (emblemId >= 12) return;

    bondLevel = UNIT_RING_BOND_LEVEL(unit);

    for (slot = 0; slot < SKILL_DEF_COUNT_SYNC; ++slot) {
        struct SkillDef sk = gSyncSkillDefs[emblemId * SKILL_DEF_COUNT_SYNC + slot];

        if (sk.kind == SKILL_EFFECT_NONE) continue;
        if (sk.tier > bondLevel) continue;

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

void ApplyEngageSkillToBattleUnit(struct BattleUnit *bu, struct Unit *unit)
{
    u8 emblemId = UNIT_RING_EMBLEM_ID(unit);

    if (emblemId == UNIT_RING_EMBLEM_NONE) return;
    if (emblemId >= 12) return;
    if (UNIT_ENGAGE_SKILL_USED(unit)) return;
    {
        struct SkillDef sk = gEngageSkillDefs[emblemId];
        UNIT_SET_ENGAGE_SKILL_USED(unit, 1);
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
    return 0;
}
