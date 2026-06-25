#include "global.h"

#include "bmskill.h"
#include "constants/skills.h"

extern CONST_DATA struct SkillData gSkillData[];
extern CONST_DATA struct ClassSkillEnt gClassSkillTable[];

static const u8* GetClassSkillList(u8 classId) {
    int i;

    for (i = 0; gClassSkillTable[i].classId != 0; ++i) {
        if (gClassSkillTable[i].classId == classId)
            return gClassSkillTable[i].skills;
    }

    return NULL;
}

void SkillDispatchBattle(enum SkillHook hook, struct SkillBattleContext* ctx) {
    const u8* skills = GetClassSkillList(ctx->attacker->unit.pClassData->number);
    int i;

    if (!skills)
        return;

    for (i = 0; skills[i] != SKILL_NONE; ++i) {
        u8 skillId = skills[i];
        const struct SkillData* skillData = gSkillData + skillId;

        if (skillData->id != skillId)
            continue;

        if (skillData->hook != hook)
            continue;

        if (skillData->battleHook)
            skillData->battleHook(ctx);
    }
}
