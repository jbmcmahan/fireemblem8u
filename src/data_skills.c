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
static void SkillAspiringHeroBattleHook(struct SkillBattleContext* ctx);

/* ---- Skill lists (SKILL_NONE-terminated) ---- */

CONST_DATA u8 SkillList_Sniper[] = {
    SKILL_SURE_SHOT,
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
    [SKILL_ASPIRING_HERO] = {
        .id = SKILL_ASPIRING_HERO,
        .hook = SKILL_HOOK_PRE_HIT,
        .battleHook = SkillAspiringHeroBattleHook,
    },
};

/* ---- Unit skill lookup tables ---- */

CONST_DATA struct UnitSkillEnt gClassSkillTable[] = {
    { CLASS_SNIPER,   SkillList_Sniper },
    { CLASS_SNIPER_F, SkillList_Sniper },
    { 0, NULL },
};

CONST_DATA struct UnitSkillEnt gCharSkillTable[] = {
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

/* ---- Stat bonus hooks (fire for every unit) ---- */

/* 4-way adjacency scan (O(1) — four tile reads) */
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

static int UnitHasAdjacentSkill(int faction, int x, int y, u8 skillId)
{
    static const int dx[] = { -1, +1,  0,  0 };
    static const int dy[] = {  0,  0, -1, +1 };
    int dir;

    for (dir = 0; dir < 4; dir++) {
        int nx = x + dx[dir];
        int ny = y + dy[dir];
        int uId;
        struct Unit *unit;
        const u8 *skills;
        int j;

        if (nx < 0 || ny < 0)
            continue;

        uId = gBmMapUnit[ny][nx];
        if (!uId)
            continue;

        if ((uId & 0xC0) != faction)
            continue;

        unit = GetUnit(uId);

        skills = GetClassSkillList(unit->pClassData->number);
        if (skills)
            for (j = 0; skills[j] != SKILL_NONE; j++)
                if (skills[j] == skillId)
                    return TRUE;

        skills = GetCharSkillList(unit->pCharacterData->number);
        if (skills)
            for (j = 0; skills[j] != SKILL_NONE; j++)
                if (skills[j] == skillId)
                    return TRUE;
    }
    return FALSE;
}

/** Alabaster Duty stat bonus.
 *
 * Fires for every unit in combat (via SkillFireStatBonusHooks).
 * Two symmetric conditions:
 *   1. Unit has SKILL_ALABASTER_DUTY and is adjacent to CA_DIVINE_DRAGON → +5
 *   2. Unit has CA_DIVINE_DRAGON and is adjacent to SKILL_ALABASTER_DUTY → +5
 */
static int HasSkillInList(const u8 *skills, u8 skillId)
{
    int j;
    if (!skills)
        return FALSE;
    for (j = 0; skills[j] != SKILL_NONE; j++)
        if (skills[j] == skillId)
            return TRUE;
    return FALSE;
}

int UnitHasSkill(const struct Unit* unit, u8 skillId)
{
    if (HasSkillInList(GetClassSkillList(unit->pClassData->number), skillId))
        return TRUE;
    if (HasSkillInList(GetCharSkillList(unit->pCharacterData->number), skillId))
        return TRUE;
    return FALSE;
}

int UnitHealStaffRangeBonus(const struct Unit *unit)
{
    return UnitHasSkill(unit, SKILL_BIG_PERSONALITY) ? 1 : 0;
}

static void SkillStatBonusAlabasterDuty(struct SkillBattleContext* ctx,
                                        const struct Unit* unit)
{
    int faction;

    if (gBattleStats.config & BATTLE_CONFIG_ARENA)
        return;

    faction = unit->index & 0xC0;

    /* Case 1: unit has SKILL_ALABASTER_DUTY → adjacent Divine Dragon? */
    if (UnitHasSkill(unit, SKILL_ALABASTER_DUTY)) {
        if (UnitHasAdjacentDivineDragon(faction, unit->xPos, unit->yPos))
            gBattleStats.critRate += 5;
        return;
    }

    /* Case 2: unit has CA_DIVINE_DRAGON → adjacent skill owner? */
    if (UNIT_CATTRIBUTES(unit) & CA_DIVINE_DRAGON) {
        if (UnitHasAdjacentSkill(faction, unit->xPos, unit->yPos,
                                 SKILL_ALABASTER_DUTY))
            gBattleStats.critRate += 5;
    }
}

/* Stat bonus registry — all hooks here fire for every unit, every combat. */

static CONST_DATA StatBonusFn sStatBonusHooks[] = {
    SkillStatBonusAlabasterDuty,
    NULL,
};

void SkillFireStatBonusHooks(struct SkillBattleContext *ctx,
                             const struct Unit *unit)
{
    int i;
    for (i = 0; sStatBonusHooks[i] != NULL; i++)
        sStatBonusHooks[i](ctx, unit);
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
    int dy2, dx2, femaleAllyCount = 0;

    if (gBattleStats.config & BATTLE_CONFIG_ARENA)
        return;

    for (dy2 = -2; dy2 <= 2; dy2++) {
        for (dx2 = -2; dx2 <= 2; dx2++) {
            int nx = x + dx2;
            int ny = y + dy2;

            if (dx2 == 0 && dy2 == 0)
                continue;

            if (ABS(dx2) + ABS(dy2) > 2)
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

static int CombatIsIsolated(const struct BattleUnit* attacker,
                            const struct BattleUnit* defender)
{
    const int dx[4] = { -1, +1,  0,  0 };
    const int dy[4] = {  0,  0, -1, +1 };
    int ax = attacker->unit.xPos, ay = attacker->unit.yPos;
    int bx = defender->unit.xPos, by = defender->unit.yPos;
    int dir;

    /* Scan attacker's 4 neighbors — reject if any unit besides defender */
    for (dir = 0; dir < 4; dir++) {
        int nx = ax + dx[dir], ny = ay + dy[dir];
        int uId;
        if (nx < 0 || ny < 0) continue;
        uId = gBmMapUnit[ny][nx];
        if (!uId) continue;
        if (nx == bx && ny == by) continue;
        return FALSE;
    }

    /* Scan defender's 4 neighbors — reject if any unit besides attacker */
    for (dir = 0; dir < 4; dir++) {
        int nx = bx + dx[dir], ny = by + dy[dir];
        int uId;
        if (nx < 0 || ny < 0) continue;
        uId = gBmMapUnit[ny][nx];
        if (!uId) continue;
        if (nx == ax && ny == ay) continue;
        return FALSE;
    }

    return TRUE;
}

/* Aspiring Hero — if no other unit is adjacent to either combatant,
 * the skill owner gets Hit +20 and Avo -10.
 *
 * Fires as PRE_HIT for the current attacker.  Both sides are checked
 * in the same call so the effect applies on counter-strikes too:
 *   - Owner attacking → hitRate += 20
 *   - Owner defending → hitRate += 10 (= defender's Avo -10) */
static void SkillAspiringHeroBattleHook(struct SkillBattleContext* ctx)
{
    if (gBattleStats.config & BATTLE_CONFIG_ARENA)
        return;

    if (!UnitHasSkill(&ctx->attacker->unit, SKILL_ASPIRING_HERO)
     && !UnitHasSkill(&ctx->defender->unit, SKILL_ASPIRING_HERO))
        return;

    if (!CombatIsIsolated(ctx->attacker, ctx->defender))
        return;

    if (UnitHasSkill(&ctx->attacker->unit, SKILL_ASPIRING_HERO))
        gBattleStats.hitRate += 20;

    if (UnitHasSkill(&ctx->defender->unit, SKILL_ASPIRING_HERO))
        gBattleStats.hitRate += 10;
}
