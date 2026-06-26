#ifndef GUARD_BMSKILL_H
#define GUARD_BMSKILL_H

#include "bmbattle.h"

enum SkillHook {
    SKILL_HOOK_PRE_HIT = 0,
};

struct SkillBattleContext {
    struct BattleUnit* attacker;
    struct BattleUnit* defender;
    struct BattleHit* hit;
};

struct SkillData {
    u8 id;
    u8 hook;
    void (*battleHook)(struct SkillBattleContext* ctx);
};

struct ClassSkillEnt {
    u8 classId;
    const u8* skills;
};

void SkillDispatchBattle(enum SkillHook hook, struct SkillBattleContext* ctx);

#endif // GUARD_BMSKILL_H
