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

/* ---- Skill lists (SKILL_NONE-terminated) ---- */

CONST_DATA u8 SkillList_Sniper[] = {
    SKILL_SURE_SHOT,
    SKILL_NONE,
};

CONST_DATA u8 CharSkillList_Louis[] = {
    SKILL_ADMIRATION,
    SKILL_NONE,
};

CONST_DATA u8 CharSkillList_Vander[] = {
    SKILL_ALABASTER_DUTY,
    SKILL_NONE,
};

CONST_DATA u8 CharSkillList_Amber[] = {
    SKILL_ASPIRING_HERO,
    SKILL_NONE,
};

CONST_DATA u8 CharSkillList_Hortensia[] = {
    SKILL_BIG_PERSONALITY,
    SKILL_NONE,
};

CONST_DATA u8 CharSkillList_Kagetsu[] = {
    SKILL_BLINDING_FLASH,
    SKILL_NONE,
};

CONST_DATA u8 CharSkillList_Panette[] = {
    SKILL_BLOOD_FURY,
    SKILL_NONE,
};

CONST_DATA u8 CharSkillList_Rosado[] = {
    SKILL_CHARMER,
    SKILL_NONE,
};

/* ---- Skill data table ---- */

CONST_DATA struct SkillData gSkillData[SKILL_MAX] = {
    [SKILL_NONE] = {
        .id = SKILL_NONE,
    },
    [SKILL_SURE_SHOT] = {
        .id = SKILL_SURE_SHOT,
        .hook = SKILL_HOOK_PRE_HIT,
        .battleHook = SkillSureShotBattleHook,
    },
    [SKILL_ADMIRATION] = {
        .id = SKILL_ADMIRATION,
    },
    [SKILL_ALABASTER_DUTY] = {
        .id = SKILL_ALABASTER_DUTY,
    },
    [SKILL_ASPIRING_HERO] = {
        .id = SKILL_ASPIRING_HERO,
    },
    [SKILL_BIG_PERSONALITY] = {
        .id = SKILL_BIG_PERSONALITY,
    },
    [SKILL_BLINDING_FLASH] = {
        .id = SKILL_BLINDING_FLASH,
    },
    [SKILL_BLOOD_FURY] = {
        .id = SKILL_BLOOD_FURY,
    },
    [SKILL_CHARMER] = {
        .id = SKILL_CHARMER,
    },
    [SKILL_CONTEMPLATIVE] = {
        .id = SKILL_CONTEMPLATIVE,
    },
};

/* ---- Unit skill lookup tables ---- */

CONST_DATA struct UnitSkillEnt gClassSkillTable[] = {
    { CLASS_SNIPER,   SkillList_Sniper },
    { CLASS_SNIPER_F, SkillList_Sniper },
    { 0, NULL },
};

CONST_DATA struct UnitSkillEnt gCharSkillTable[] = {
    { CHARACTER_LOUIS,     CharSkillList_Louis },
    { CHARACTER_VANDER,    CharSkillList_Vander },
    { CHARACTER_AMBER,     CharSkillList_Amber },
    { CHARACTER_HORTENSIA, CharSkillList_Hortensia },
    { CHARACTER_KAGETSU,   CharSkillList_Kagetsu },
    { CHARACTER_PANETTE,   CharSkillList_Panette },
    { CHARACTER_ROSADO,    CharSkillList_Rosado },
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

/* ---- Prediction-visible battle stat bonuses ---- */

static int IsMapPositionInBounds(int x, int y)
{
    return x >= 0 && y >= 0 && x < gBmMapSize.x && y < gBmMapSize.y;
}

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

        if (!IsMapPositionInBounds(nx, ny))
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

        if (!IsMapPositionInBounds(nx, ny))
            continue;

        uId = gBmMapUnit[ny][nx];
        if (!uId)
            continue;

        if ((uId & 0xC0) != faction)
            continue;

        unit = GetUnit(uId);
        if (UnitHasSkill(unit, skillId))
            return TRUE;
    }
    return FALSE;
}

static int UnitHasFemaleAlliesWithinRange(const struct Unit *unit, int range, int required)
{
    int x = unit->xPos;
    int y = unit->yPos;
    int faction = unit->index & 0xC0;
    int count = 0;
    int dy, dx;

    for (dy = -range; dy <= range; dy++) {
        for (dx = -range; dx <= range; dx++) {
            int nx = x + dx;
            int ny = y + dy;
            int uId;
            struct Unit *other;

            if (dx == 0 && dy == 0)
                continue;

            if (ABS(dx) + ABS(dy) > range)
                continue;

            if (!IsMapPositionInBounds(nx, ny))
                continue;

            uId = gBmMapUnit[ny][nx];
            if (!uId)
                continue;

            if ((uId & 0xC0) != faction)
                continue;

            other = GetUnit(uId);

            if (other->state & (US_DEAD | US_NOT_DEPLOYED | US_BIT16))
                continue;

            if (UNIT_CATTRIBUTES(other) & CA_FEMALE) {
                count++;
                if (count >= required)
                    return TRUE;
            }
        }
    }

    return FALSE;
}

static void AddBattleAvoid(struct BattleUnit *bu, int amount)
{
    bu->battleAvoidRate += amount;

    if (bu->battleAvoidRate < 0)
        bu->battleAvoidRate = 0;
}

static void AddBattleCrit(struct BattleUnit *bu, int amount)
{
    bu->battleCritRate += amount;

    if (bu->battleCritRate < 0)
        bu->battleCritRate = 0;
}

static void ApplyUnitBattleStatBonuses(struct BattleUnit *bu)
{
    const struct Unit *unit = &bu->unit;
    int faction = unit->index & 0xC0;

    if (UnitHasSkill(unit, SKILL_ADMIRATION)) {
        if (UnitHasFemaleAlliesWithinRange(unit, 2, 2))
            bu->battleDefense += 2;
    }

    if (UnitHasSkill(unit, SKILL_ALABASTER_DUTY)) {
        if (UnitHasAdjacentDivineDragon(faction, unit->xPos, unit->yPos))
            bu->battleCritRate += 5;
    } else if (UNIT_CATTRIBUTES(unit) & CA_DIVINE_DRAGON) {
        if (UnitHasAdjacentSkill(faction, unit->xPos, unit->yPos,
                                 SKILL_ALABASTER_DUTY))
            bu->battleCritRate += 5;
    }

    if (UnitHasSkill(unit, SKILL_BLOOD_FURY) && unit->curHP < unit->maxHP)
        bu->battleCritRate += 10;

    if (SkillUnitHasContemplativeBonus(unit))
        bu->battleDefense += 2;
}

static int CombatIsIsolated(const struct BattleUnit* attacker,
                            const struct BattleUnit* defender)
{
    const int dx[4] = { -1, +1,  0,  0 };
    const int dy[4] = {  0,  0, -1, +1 };
    int ax = attacker->unit.xPos, ay = attacker->unit.yPos;
    int bx = defender->unit.xPos, by = defender->unit.yPos;
    int dir;

    for (dir = 0; dir < 4; dir++) {
        int nx = ax + dx[dir], ny = ay + dy[dir];
        int uId;

        if (!IsMapPositionInBounds(nx, ny))
            continue;

        uId = gBmMapUnit[ny][nx];
        if (!uId)
            continue;

        if (nx == bx && ny == by)
            continue;

        return FALSE;
    }

    for (dir = 0; dir < 4; dir++) {
        int nx = bx + dx[dir], ny = by + dy[dir];
        int uId;

        if (!IsMapPositionInBounds(nx, ny))
            continue;

        uId = gBmMapUnit[ny][nx];
        if (!uId)
            continue;

        if (nx == ax && ny == ay)
            continue;

        return FALSE;
    }

    return TRUE;
}

void SkillApplyBattleStatBonuses(struct BattleUnit *actor,
                                 struct BattleUnit *target)
{
    if (gBattleStats.config & BATTLE_CONFIG_ARENA)
        return;

    if (!actor)
        return;

    ApplyUnitBattleStatBonuses(actor);

    if (!target || !target->unit.pClassData)
        return;

    ApplyUnitBattleStatBonuses(target);

    if (UnitHasSkill(&actor->unit, SKILL_BLINDING_FLASH))
        AddBattleAvoid(target, -10);

    if (UnitHasSkill(&actor->unit, SKILL_CHARMER) &&
        SkillUnitFoughtMostRecentOpponent(&actor->unit, &target->unit))
        AddBattleCrit(target, -10);

    if (UnitHasSkill(&target->unit, SKILL_CHARMER) &&
        SkillUnitFoughtMostRecentOpponent(&target->unit, &actor->unit))
        AddBattleCrit(actor, -10);

    if (!CombatIsIsolated(actor, target))
        return;

    if (UnitHasSkill(&actor->unit, SKILL_ASPIRING_HERO)) {
        actor->battleHitRate += 20;
        AddBattleAvoid(actor, -10);
    }

    if (UnitHasSkill(&target->unit, SKILL_ASPIRING_HERO)) {
        target->battleHitRate += 20;
        AddBattleAvoid(target, -10);
    }
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
