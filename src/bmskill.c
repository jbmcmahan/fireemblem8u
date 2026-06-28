#include "global.h"

#include "bmskill.h"
#include "constants/skills.h"

extern CONST_DATA struct SkillData gSkillData[];
extern CONST_DATA struct UnitSkillEnt gClassSkillTable[];
extern CONST_DATA struct UnitSkillEnt gCharSkillTable[];

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

int UnitHealStaffRangeBonus(const struct Unit *unit)
{
    return UnitHasSkill(unit, SKILL_BIG_PERSONALITY) ? 1 : 0;
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
