#ifndef GUARD_CONSTANTS_SKILLS_H
#define GUARD_CONSTANTS_SKILLS_H

/* Maximum character skill ID — keep in sync with gSkillData[] size.
 * SKILL_BIG_PERSONALITY = 5 means this must be at least 6.
 */
enum { SKILL_MAX = 6 };

enum {
    SKILL_NONE            = 0,
    SKILL_SURE_SHOT       = 1,
    SKILL_ADMIRATION      = 2,
    SKILL_ALABASTER_DUTY  = 3,
    SKILL_ASPIRING_HERO   = 4,
    SKILL_BIG_PERSONALITY = 5,
};

#endif // GUARD_CONSTANTS_SKILLS_H
