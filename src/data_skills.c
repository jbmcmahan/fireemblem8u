#include "global.h"

#include "bmskill.h"
#include "bmitem.h"
#include "constants/items.h"
#include "constants/classes.h"
#include "constants/skills.h"

static void SkillSureShotBattleHook(struct SkillBattleContext* ctx);

CONST_DATA u8 SkillList_Sniper[] = {
    SKILL_SURE_SHOT,
    SKILL_NONE,
};

CONST_DATA struct SkillData gSkillData[] = {
    [SKILL_SURE_SHOT] = {
        .id = SKILL_SURE_SHOT,
        .hook = SKILL_HOOK_PRE_HIT,
        .battleHook = SkillSureShotBattleHook,
    },
};

CONST_DATA struct ClassSkillEnt gClassSkillTable[] = {
    { CLASS_SNIPER, SkillList_Sniper },
    { CLASS_SNIPER_F, SkillList_Sniper },
    { 0, NULL },
};

static void SkillSureShotBattleHook(struct SkillBattleContext* ctx) {
    struct BattleHit* hit = ctx->hit;
    struct BattleUnit* attacker = ctx->attacker;

    if (hit->attributes & BATTLE_HIT_ATTR_SURESHOT)
        return;

    if (hit->attributes & BATTLE_HIT_ATTR_PIERCE)
        return;

    if (hit->attributes & BATTLE_HIT_ATTR_GREATSHLD)
        return;

    switch (GetItemIndex(attacker->weapon)) {
    case ITEM_BALLISTA_REGULAR:
    case ITEM_BALLISTA_LONG:
    case ITEM_BALLISTA_KILLER:
        return;
    }

    if (BattleRoll1RN(attacker->unit.level, FALSE) == TRUE)
        hit->attributes |= BATTLE_HIT_ATTR_SURESHOT;
}
