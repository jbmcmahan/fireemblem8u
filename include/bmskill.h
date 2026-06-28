#ifndef GUARD_BMSKILL_H
#define GUARD_BMSKILL_H

#include "bmbattle.h"

/* ---- Skill dispatch (ownership-based) ---- */

enum { MAX_UNIT_SKILL_SOURCES = 4 };

enum SkillHook {
    SKILL_HOOK_PRE_HIT   = 0,
    SKILL_HOOK_AFTER_DMG = 1,
};

struct SkillBattleContext {
    struct BattleUnit *attacker;
    struct BattleUnit *defender;
    struct BattleHit  *hit;
};

struct SkillData {
    u8 id;
    u8 hook;
    void (*battleHook)(struct SkillBattleContext *ctx);
};

struct UnitSkillEnt {
    u8 unitId;
    const u8 *skills;   /* SKILL_NONE-terminated */
};

extern CONST_DATA struct UnitSkillEnt gClassSkillTable[];
extern CONST_DATA struct UnitSkillEnt gCharSkillTable[];

int GetUnitSkills(const struct Unit *unit,
                  const u8 **outLists, int max);

void SkillDispatchBattle(enum SkillHook hook,
                         struct SkillBattleContext *ctx,
                         const u8 *skillList);

void SkillDispatchForUnit(enum SkillHook hook,
                          struct SkillBattleContext *ctx,
                          const struct Unit *unit);

void SkillApplyBattleStatBonuses(struct BattleUnit *actor,
                                 struct BattleUnit *target);

int UnitHealStaffRangeBonus(const struct Unit *unit);

/** Does *unit own *skillId* (class or character tables)? */
s8 UnitHasSkill(const struct Unit *unit, u8 skillId);

#endif // GUARD_BMSKILL_H
