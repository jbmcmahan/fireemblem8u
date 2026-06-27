#include "global.h"

#include "bmskill.h"
#include "bmitem.h"
#include "bmmap.h"
#include "bmunit.h"
#include "constants/items.h"
#include "constants/classes.h"
#include "constants/characters.h"
#include "constants/skills.h"

static void SkillSureShotBattleHook(struct SkillBattleContext* ctx);
static void SkillAdmirationBattleHook(struct SkillBattleContext* ctx);

/* ---- Skill lists (SKILL_NONE-terminated) ---- */

CONST_DATA u8 SkillList_Sniper[] = {
    SKILL_SURE_SHOT,
    SKILL_NONE,
};

CONST_DATA u8 CharSkillList_Louis[] = {
    SKILL_ADMIRATION,
    SKILL_NONE,
};

/* ---- Skill data table ---- */

CONST_DATA struct SkillData gSkillData[] = {
    [SKILL_SURE_SHOT] = {
        .id = SKILL_SURE_SHOT,
        .hook = SKILL_HOOK_PRE_HIT,
        .battleHook = SkillSureShotBattleHook,
    },
    [SKILL_ADMIRATION] = {
        .id = SKILL_ADMIRATION,
        .hook = SKILL_HOOK_AFTER_DMG,
        .battleHook = SkillAdmirationBattleHook,
    },
};

/* ---- Unit skill lookup tables ---- */
/*
 * Each table maps a unit identifier to a skill list.  New tables can be
 * added for any ownership model (character, class, learned, equipment,
 * etc.).  The dispatcher only sees the resolved skill list.
 */

CONST_DATA struct UnitSkillEnt gClassSkillTable[] = {
    { CLASS_SNIPER,   SkillList_Sniper },
    { CLASS_SNIPER_F, SkillList_Sniper },
    { 0, NULL },
};

CONST_DATA struct UnitSkillEnt gCharSkillTable[] = {
    { CHARACTER_LOUIS, CharSkillList_Louis },
    { 0, NULL },
};

const u8* GetClassSkillList(u8 classId)
{
    int i;
    for (i = 0; gClassSkillTable[i].unitId != 0; ++i) {
        if (gClassSkillTable[i].unitId == classId)
            return gClassSkillTable[i].skills;
    }
    return NULL;
}

const u8* GetCharSkillList(u8 charId)
{
    int i;
    for (i = 0; gCharSkillTable[i].unitId != 0; ++i) {
        if (gCharSkillTable[i].unitId == charId)
            return gCharSkillTable[i].skills;
    }
    return NULL;
}

/* ---- Hook implementations ---- */

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

static void SkillAdmirationBattleHook(struct SkillBattleContext* ctx) {
    int x = ctx->defender->unit.xPos;
    int y = ctx->defender->unit.yPos;
    int faction = ctx->defender->unit.index & 0xC0;
    int i, dx, dy, femaleAllyCount = 0;

    if (gBattleStats.config & BATTLE_CONFIG_ARENA)
        return;

    for (dy = -2; dy <= 2; dy++) {
        for (dx = -2; dx <= 2; dx++) {
            int nx = x + dx;
            int ny = y + dy;

            if (dx == 0 && dy == 0)
                continue;

            if (ABS(dx) + ABS(dy) > 2)
                continue;

            int uId = gBmMapUnit[ny][nx];
            struct Unit* unit;

            if (!uId)
                continue;

            unit = GetUnit(uId);

            if ((uId & 0xC0) != faction)
                continue;

            if (unit->state & (US_DEAD | US_NOT_DEPLOYED | US_BIT16))
                continue;

            if (unit == &ctx->defender->unit)
                continue;

            if (UNIT_CATTRIBUTES(unit) & CA_FEMALE)
                femaleAllyCount++;
        }
    }

    if (femaleAllyCount >= 2 && gBattleStats.damage >= 2)
        gBattleStats.damage -= 2;
}
