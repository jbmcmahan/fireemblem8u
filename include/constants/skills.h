#ifndef GUARD_CONSTANTS_SKILLS_H
#define GUARD_CONSTANTS_SKILLS_H

/* Maximum character skill ID — keep in sync with gSkillData[] size.
 * SKILL_ASPIRING_HERO = 4 means this must be at least 5.
 */
enum { SKILL_MAX = 5 };

enum {
    SKILL_NONE            = 0,
    SKILL_SURE_SHOT       = 1,
    SKILL_ADMIRATION      = 2,
    SKILL_ALABASTER_DUTY  = 3,
    SKILL_ASPIRING_HERO   = 4,
};

#endif // GUARD_CONSTANTS_SKILLS_H
