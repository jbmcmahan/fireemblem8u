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
static void SkillAlabasterDutyBattleHook(struct SkillBattleContext* ctx);

/* ---- Skill lists (SKILL_NONE-terminated) ---- */

CONST_DATA u8 SkillList_Sniper[] = {
    SKILL_SURE_SHOT,
    SKILL_NONE,
};

/* Vander's personal skill: grants +5 crit when adjacent to the Divine
 * Dragon.  Vander has Alabaster Duty; the Divine Dragon has CA_DIVINE_DRAGON. */
CONST_DATA u8 CharSkillList_Vander[] = {
    SKILL_ALABASTER_DUTY,
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
    [SKILL_ALABASTER_DUTY] = {
        .id = SKILL_ALABASTER_DUTY,
        .hook = SKILL_HOOK_PRE_CRIT,
        .battleHook = SkillAlabasterDutyBattleHook,
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
    { CHARACTER_LOUIS,  CharSkillList_Louis  },
    { CHARACTER_VANDER, CharSkillList_Vander },
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

/* ---- Helper: scan 4-way adjacency for CA_DIVINE_DRAGON ally -------- */

static int UnitHasAdjacentDivineDragon(int faction, int x, int y)
{
    static const int dx[] = { -1, +1,  0,  0 };
    static const int dy[] = {  0,  0, -1, +1 };
    int dir;

    for (dir = 0; dir < 4; dir++) {
        int nx = x + dx[dir];
        int ny = y + dy[dir];
        int uId;
        struct Unit *unit;

        if (nx < 0 || ny < 0)
            continue;

        uId = gBmMapUnit[ny][nx];
        if (!uId)
            continue;

        if ((uId & 0xC0) != faction)
            continue;

        unit = GetUnit(uId);
        if (UNIT_CATTRIBUTES(unit) & CA_DIVINE_DRAGON)
            return TRUE;
    }

    return FALSE;
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

/* Alabaster Duty — Vander's personal skill.
 *
 * Two cases:
 *   1. Vander (has this skill) attacks an enemy.  If an ally with
 *      CA_DIVINE_DRAGON is within 1 space of Vander, Vander gets +5 crit.
 *   2. A unit with CA_DIVINE_DRAGON (the Divine Dragon) attacks an enemy.
 *      If an ally with Alabaster Duty (Vander) is within 1 space, the
 *      Divine Dragon gets +5 crit.
 *
 * The hook fires once per dispatch (attacker or defender).  It checks
 * BOTH the attacker and the defender: if either one qualifies, it
 * boosts only that unit's crit for this exchange.  For the attacker
 * it modifies gBattleStats.critRate directly.  For the defender it
 * modifies battleEffectiveCritRate so the counter-attack inherits
 * the bonus via BattleUpdateBattleStats. */
static void SkillAlabasterDutyBattleHook(struct SkillBattleContext* ctx) {
    int faction;

    if (gBattleStats.config & BATTLE_CONFIG_ARENA)
        return;

    /* Case 1: attacker has this skill (Vander) and is adjacent to the Divine Dragon */
    if (ctx->attacker->unit.pCharacterData->number == CHARACTER_VANDER) {
        faction = ctx->attacker->unit.index & 0xC0;
        if (UnitHasAdjacentDivineDragon(faction,
                ctx->attacker->unit.xPos, ctx->attacker->unit.yPos)) {
            gBattleStats.critRate += 5;
        }
    }
    /* Case 2: defender has this skill (Vander) and is adjacent to the Divine Dragon */
    if (ctx->defender->unit.pCharacterData->number == CHARACTER_VANDER) {
        faction = ctx->defender->unit.index & 0xC0;
        if (UnitHasAdjacentDivineDragon(faction,
                ctx->defender->unit.xPos, ctx->defender->unit.yPos)) {
            ctx->defender->battleEffectiveCritRate += 5;
        }
    }
}
