#include "engage_mechanics/engage_skills.h"

#define NO_RING 0xFF

void ApplySyncSkillsToBattleUnit(struct BattleUnit *bu, struct Unit *unit)
{
    u8 emblemId = unit->ringEmblemId;
    if (emblemId == NO_RING) return;
    {
        u8 bondLevel = unit->ringBondLevel;
        for (u8 slot = 0; slot < SKILL_DEF_COUNT_SYNC; ++slot) {
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
}

void ApplyEngageSkillToBattleUnit(struct BattleUnit *bu, struct Unit *unit)
{
    if (unit->ringEmblemId == NO_RING) return;
    if (unit->uEngageSkillUsed) return;
    {
        struct SkillDef sk = gEngageSkillDefs[unit->ringEmblemId];
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
    return 0;
}