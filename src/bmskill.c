#include "global.h"

#include "bmskill.h"
#include "constants/skills.h"

extern CONST_DATA struct SkillData gSkillData[];
extern CONST_DATA struct UnitSkillEnt gClassSkillTable[];
extern CONST_DATA struct UnitSkillEnt gCharSkillTable[];

static u8 sUnitMostRecentOpponent[0x100] __attribute__((section(".bss")));
static u8 sUnitContemplativeActive[(0x100 + 7) / 8] __attribute__((section(".bss")));

static u8 GetUnitSkillStateId(const struct Unit *unit)
{
    if (!unit)
        return 0;

    return unit->index;
}

static const u8 *findInTable(const struct UnitSkillEnt *table, u8 unitId)
{
    int i;
    for (i = 0; table[i].unitId != 0; ++i)
        if (table[i].unitId == unitId)
            return table[i].skills;
    return NULL;
}

int GetUnitSkills(const struct Unit *unit,
                  const u8 **outLists, int max)
{
    int count = 0;
    const u8 *s;

    if (!unit)
        return 0;

    /* Class skills */
    if (unit->pClassData) {
        s = findInTable(gClassSkillTable, unit->pClassData->number);
        if (s && count < max) outLists[count++] = s;
    }

    /* Character (personal) skills */
    if (unit->pCharacterData) {
        s = findInTable(gCharSkillTable, unit->pCharacterData->number);
        if (s && count < max) outLists[count++] = s;
    }

    return count;
}

static s8 HasSkillInList(const u8 *skills, u8 skillId)
{
    int i;

    if (!skills)
        return FALSE;

    for (i = 0; skills[i] != SKILL_NONE; ++i)
        if (skills[i] == skillId)
            return TRUE;

    return FALSE;
}

s8 UnitHasSkill(const struct Unit *unit, u8 skillId)
{
    const u8 *lists[MAX_UNIT_SKILL_SOURCES];
    int i, count;

    if (!unit)
        return FALSE;

    count = GetUnitSkills(unit, lists, MAX_UNIT_SKILL_SOURCES);

    for (i = 0; i < count; ++i)
        if (HasSkillInList(lists[i], skillId))
            return TRUE;

    return FALSE;
}

static void SetUnitContemplativeState(u8 id, s8 active)
{
    u8 mask;

    if (!id)
        return;

    mask = 1 << (id & 7);

    if (active)
        sUnitContemplativeActive[id >> 3] |= mask;
    else
        sUnitContemplativeActive[id >> 3] &= ~mask;
}

static s8 GetUnitContemplativeState(u8 id)
{
    if (!id)
        return FALSE;

    return (sUnitContemplativeActive[id >> 3] >> (id & 7)) & 1;
}

int UnitHealStaffRangeBonus(const struct Unit *unit)
{
    return UnitHasSkill(unit, SKILL_BIG_PERSONALITY) ? 1 : 0;
}

void SkillRecordBattleOpponents(const struct Unit *unitA,
                                const struct Unit *unitB)
{
    u8 idA = GetUnitSkillStateId(unitA);
    u8 idB = GetUnitSkillStateId(unitB);

    if (!idA || !idB)
        return;

    sUnitMostRecentOpponent[idA] = idB;
    sUnitMostRecentOpponent[idB] = idA;
}

void SkillClearUnitCombatState(const struct Unit *unit)
{
    int i;
    u8 id = GetUnitSkillStateId(unit);

    if (!id)
        return;

    sUnitMostRecentOpponent[id] = 0;
    SetUnitContemplativeState(id, FALSE);

    for (i = 1; i < 0x100; i++) {
        if (sUnitMostRecentOpponent[i] == id)
            sUnitMostRecentOpponent[i] = 0;
    }
}

s8 SkillUnitFoughtMostRecentOpponent(const struct Unit *unit,
                                     const struct Unit *opponent)
{
    u8 id = GetUnitSkillStateId(unit);
    u8 opponentId = GetUnitSkillStateId(opponent);

    if (!id || !opponentId)
        return FALSE;

    return sUnitMostRecentOpponent[id] == opponentId;
}

void SkillOnUnitBeginAction(const struct Unit *unit)
{
    u8 id = GetUnitSkillStateId(unit);

    if (!id)
        return;

    SetUnitContemplativeState(id, FALSE);
}

void SkillOnUnitWait(const struct Unit *unit)
{
    u8 id = GetUnitSkillStateId(unit);

    if (!id)
        return;

    if (!UnitHasSkill(unit, SKILL_CONTEMPLATIVE))
        return;

    SetUnitContemplativeState(id, TRUE);
}

s8 SkillUnitHasContemplativeBonus(const struct Unit *unit)
{
    u8 id = GetUnitSkillStateId(unit);

    if (!id)
        return FALSE;

    return GetUnitContemplativeState(id);
}

void SkillDispatchBattle(enum SkillHook hook,
                         struct SkillBattleContext *ctx,
                         const u8 *skillList)
{
    int i;

    if (!skillList)
        return;

    for (i = 0; skillList[i] != SKILL_NONE; ++i) {
        u8 skillId = skillList[i];
        const struct SkillData *sd;

        if (skillId >= SKILL_MAX)
            continue;

        sd = gSkillData + skillId;

        if (sd->id != skillId)
            continue;

        if (sd->hook != hook)
            continue;

        if (sd->battleHook)
            sd->battleHook(ctx);
    }
}

void SkillDispatchForUnit(enum SkillHook hook,
                          struct SkillBattleContext *ctx,
                          const struct Unit *unit)
{
    const u8 *lists[MAX_UNIT_SKILL_SOURCES];
    int n, i;

    n = GetUnitSkills(unit, lists, MAX_UNIT_SKILL_SOURCES);

    for (i = 0; i < n; ++i)
        SkillDispatchBattle(hook, ctx, lists[i]);
}
