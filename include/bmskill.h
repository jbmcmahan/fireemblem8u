#ifndef GUARD_BMSKILL_H
#define GUARD_BMSKILL_H

#include "bmbattle.h"

/* Maximum distinct skill lists a single unit can resolve (class,
 * character, learned, equipment, …).  Increase if a new source is added. */
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

/* Maps a unit identifier to a SKILL_NONE-terminated skill list.
 * Each ownership kind (class, character, …) gets its own CONST_DATA
 * table; all share this shape. */
struct UnitSkillEnt {
    u8 unitId;
    const u8 *skills;
};

/* Ownership tables — append new ones as skill sources grow. */
extern CONST_DATA struct UnitSkillEnt gClassSkillTable[];
extern CONST_DATA struct UnitSkillEnt gCharSkillTable[];

/** Resolve all skill lists for *unit*.  Writes up to *max* pointers
 *  into *outLists*.  Returns the number written. */
int GetUnitSkills(const struct Unit *unit,
                  const u8 **outLists, int max);

/** Dispatch one skill list: fire every hook matching *hook*. */
void SkillDispatchBattle(enum SkillHook hook,
                         struct SkillBattleContext *ctx,
                         const u8 *skillList);

/** Dispatch all skill lists for *unit*.  Convenience wrapper around
 *  GetUnitSkills + SkillDispatchBattle. */
void SkillDispatchForUnit(enum SkillHook hook,
                          struct SkillBattleContext *ctx,
                          const struct Unit *unit);

#endif // GUARD_BMSKILL_H
