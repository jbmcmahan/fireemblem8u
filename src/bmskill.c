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

    /* Append new sources here as skill kinds grow.
     * Each table maps ONE kind of identifier. */

    /* Class skills */
    s = findInTable(gClassSkillTable, unit->pClassData->number);
    if (s && count < max) outLists[count++] = s;

    /* Character (personal) skills */
    s = findInTable(gCharSkillTable, unit->pCharacterData->number);
    if (s && count < max) outLists[count++] = s;

    return count;
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
        const struct SkillData *sd = gSkillData + skillId;

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
